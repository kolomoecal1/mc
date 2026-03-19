#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float temperature;
    float pressure;
    float humidity;
} bme280_data_t;

bool bme280_wrapper_init(void);

bme280_data_t bme280_wrapper_read(void);

void bme280_wrapper_get_raw(int32_t* temp_raw, int32_t* pres_raw, int32_t* hum_raw);