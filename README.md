# VL53L1

Esse repositório contém uma biblioteca para lidar com o sensor de distância [VL53L1](https://www.st.com/en/imaging-and-photonics-solutions/vl53l1x.html) da ST.

Essa biblioteca foi feita para ser utilizadas como submódulo no [STM32ProjectTemplate](https://github.com/ThundeRatz/STM32ProjectTemplate).

## Adicionando o submódulo ao projeto

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

## Utilizando a biblioteca

Para utilizar a biblioteca, é necessário que, para cada sensor utlizado, sejam criadas as seguintes variáveis.

```C
VL53L1_Dev_t device;
VL53L1_RangingMeasurementData_t ranging_data;
VL53L1_CalibrationData_t calibration;
```

Cada sensor deve ser inciializado separadamente com a função:

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


---------------------

Equipe ThundeRatz de Robótica
