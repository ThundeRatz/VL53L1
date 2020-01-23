# VL53L1

Esse repositório contém uma biblioteca para lidar com o sensor de distância [VL53L1](https://www.st.com/en/imaging-and-photonics-solutions/vl53l1x.html) da ST.

Essa biblioteca foi feita para ser utilizada como submódulo no [STM32ProjectTemplate](https://github.com/ThundeRatz/STM32ProjectTemplate).

# Índice

- [Índice](#índice)
- [Utilizando a Biblioteca](#utilizando-a-biblioteca)
  - [Adicionando o Submódulo ao Projeto](#adicionando-o-submódulo-ao-projeto)
  - [Guia de Utilização](#guia-de-utilização)
    - [Instruções Básicas](#instruções-básicas)
    - [Configurações Adicionais](#configurações-adicionais)
    - [Exemplo de Adaptação da Biblioteca](#exemplo-de-adaptação-da-biblioteca-para-3-sensores-vl53l1)
- [Guia de Funcionamento da API](#guia-de-funcionamento-da-api)
  - [Sequências de Utilização](#sequências-de-utilização)

# Utilizando a Biblioteca

## Adicionando o Submódulo ao Projeto

Crie um diretório chamado `lib`, caso não exista:

```bash
mkdir lib
```
E adicione o submódulo fazendo:

* Com HTTPS:
```bash
git submodule add --name VL53L1 https://github.com/ThundeRatz/VL53L1.git lib/VL53L1
```

* Com SSH:
```bash
git submodule add --name VL53L1 git@github.com:ThundeRatz/VL53L1.git lib/VL53L1
```

## Guia de Utilização

### Instruções Básicas

Para utilizar a biblioteca, é necessário que, para cada sensor utlizado, sejam criadas as seguintes variáveis.

```C
VL53L1_Dev_t device;
VL53L1_RangingMeasurementData_t ranging_data;
VL53L1_CalibrationData_t calibration;
```

Cada sensor deve ser inicializado separadamente com a função:

```C
VL53L1_Error vl53l1_init(VL53L1_Dev_t* p_device, VL53L1_CalibrationData_t* calibration);
```

Quando utilizar mais de um sensor na mesma aplicação, é necessário desligar todos incialmente, e, individualmente, ligar, trocar o endereço de I2C e reiniciar cada um. Esse processo é feito com as funções abaixo, seguidas pela função de init.

```C
void vl53l1_turn_off(VL53L1_Dev_t* p_device);

void vl53l1_turn_on(VL53L1_Dev_t* p_device);

VL53L1_API VL53L1_Error VL53L1_SetDeviceAddress(VL53L1_DEV Dev, uint8_t DeviceAddress);
```

É importante que o usuário verifique o valor retornado por todas as funções acima. Valores diferentes de 0 indicam má inicialização de algum sensor, o que compromete a incialização dos subsequentes e pode causar um comportamento estranho em todas as leituras.

E finalmente, para atualizar a leitura do sensor, utiliza-se a função:

```C
uint8_t vl53l1_update_reading(VL53L1_Dev_t* p_device, VL53L1_RangingMeasurementData_t* p_ranging_data, uint16_t* p_reading,
                             uint16_t max_range);
```

Onde ```uint16_t* p_reading``` armazena o valor da leitura.

### Configurações Adicionais

Caso conveniente, o usuário pode configurar algumas propriedades opcionais dos sensores, como o Modo de Distância e a Expensa de Tempo. Essas configurações podem ser configuradas automaticamente pela função ```vl53l1_set_default_config(VL53L1_Dev_t* p_device)```, ou manualmente pelo usuário como uma atribuição à estrutura ```VL53L1_Dev_t device```. Explicações mais detalhadas desses atributos estão presentes a seguir:

#### Modo de Distância

O Modo de Distância configura propriedades internas do dispositivo dependendo do alcance desejado pelo usuário. Ele pode ser atribuído os seguintes valores:

* VL53L1_DISTANCEMODE_SHORT   (Até 1.3 m)
* VL53L1_DISTANCEMODE_MEDIUM  (Até 3 m)
* VL53L1_DISTANCEMODE_LONG    (Até 4 m)

Por exemplo, caso o uso idealizado do sensor demande um alcance de 2 m, a atribuição do Modo de Distância poderia ser feita da seguinte forma:

```C
device.distance_mode = VL53L1_DISTANCEMODE_MEDIUM;
```

#### Expensa de Tempo

A Expensa de Tempo é o tempo que o sensor gastará para realizar uma medida de distância. Quanto maior a Expensa de Tempo, maior o alcance e a precisão da medida, às custas de um atraso maior na taxa de atualização. O usuário pode configurar o valor desse intervalo de tempo de 20 ms a 1000 ms.

Caso seja desejada uma Expensa de Tempo de 66 ms, a configuração seria a seguinte:

```C
device.timing_budget_us = 66000; // Medida em microssegundos
```

### Exemplo de adaptação da biblioteca para 3 sensores VL53L1.

Um projeto exemplo pode ser encontrado em [Berbardo/VL53L1_Example](https://github.com/Berbardo/VL53L1_Example), feito com base no [STM32ProjectTemplate](https://github.com/ThundeRatz/STM32ProjectTemplate).

# Guia de Funcionamento da API

Esta biblioteca utiliza a [API completa do VL53L1](https://www.st.com/en/embedded-software/stsw-img007.html) para adaptá-lo, que possui uma série de funções úteis para a utilização e modificação do sensor. A função dessa seção é familiarizar o usuário a essas funções para aproveitar o máximo dos sensores.

## Sequências de Utilização

O [Guia da API](docs/VL53L1X_API_User_Manual.pdf) nos fornece orientações quanto à sequência de utilização dos sensores, indicando a ordem que devemos chamar cada função de inicialização, calibração e mensuração dos sensores. Essas sequências estão presentes a seguir, e serão explicadas em detalhes na próxima seção.

### Sequência de Calibração

![Calibration Flow](docs/Calibration_Flow.png)

### Sequência de Mensuração

![Ranging Flow](docs/Ranging_Flow.png)

---------------------

Equipe ThundeRatz de Robótica
