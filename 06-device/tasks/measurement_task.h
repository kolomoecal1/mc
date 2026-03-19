#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../sensors/bme280_wrapper.h"

#define HISTORY_SIZE 60

typedef struct {
    float temperature[HISTORY_SIZE];
    float pressure[HISTORY_SIZE];
    float humidity[HISTORY_SIZE];
    uint16_t index;
    uint16_t count;
    uint32_t last_measurement_ms;
    uint32_t period_ms;
    bool enabled;
} measurement_history_t;

// Прототипы функций
void measurement_task_init(uint32_t period_ms);
void measurement_set_period(uint32_t period_ms);
uint32_t measurement_get_period(void);
void measurement_set_enabled(bool enabled);
bool measurement_is_enabled(void);
bme280_data_t measurement_get_last(void);
const measurement_history_t* measurement_get_history(void);
void measurement_task_handler(void);