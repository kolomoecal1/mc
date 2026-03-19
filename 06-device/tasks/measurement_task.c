#include "measurement_task.h"
#include <string.h>
#include "pico/stdlib.h"

static measurement_history_t history = {0};
static bme280_data_t last_data = {0};
static uint32_t current_period_ms = 1000;
static bool measurements_enabled = true;

void measurement_task_init(uint32_t period_ms)
{
    current_period_ms = period_ms;
    history.index = 0;
    history.count = 0;
    history.enabled = true;
    history.last_measurement_ms = 0;
    
    memset(history.temperature, 0, sizeof(history.temperature));
    memset(history.pressure, 0, sizeof(history.pressure));
    memset(history.humidity, 0, sizeof(history.humidity));
    bme280_data_t data = bme280_wrapper_read();
    last_data = data;
    
    history.temperature[0] = data.temperature;
    history.pressure[0] = data.pressure;
    history.humidity[0] = data.humidity;
    history.index = 1;
    history.count = 1;
    history.last_measurement_ms = time_us_32() / 1000;
}

void measurement_set_period(uint32_t period_ms)
{
    if (period_ms < 100) period_ms = 100;
    if (period_ms > 60000) period_ms = 60000;
    current_period_ms = period_ms;
}

uint32_t measurement_get_period(void)
{
    return current_period_ms;
}

void measurement_set_enabled(bool enabled)
{
    measurements_enabled = enabled;
    history.enabled = enabled;
}

bool measurement_is_enabled(void)
{
    return measurements_enabled;
}

bme280_data_t measurement_get_last(void)
{
    return last_data;
}

const measurement_history_t* measurement_get_history(void)
{
    return &history;
}

void measurement_task_handler(void)
{
    if (!measurements_enabled) return;
    uint32_t now_ms = time_us_32() / 1000;
    if (now_ms - history.last_measurement_ms >= current_period_ms)
    {
        bme280_data_t data = bme280_wrapper_read();
        last_data = data;
        history.temperature[history.index] = data.temperature;
        history.pressure[history.index] = data.pressure;
        history.humidity[history.index] = data.humidity;
        
        history.index = (history.index + 1) % HISTORY_SIZE;
        if (history.count < HISTORY_SIZE)
        {
            history.count++;
        }
        
        history.last_measurement_ms = now_ms;
    }
}