set(VL53L1_PATH
    ./lib/VL53L1
)

list(APPEND LIB_SOURCES
    ${VL53L1_PATH}/src/vl53l1.c
)

list(APPEND LIB_INCLUDE_DIRECTORIES
    ${VL53L1_PATH}/inc
)