#include "bme280_wrapper.h"
#include "bme280-driver.h"
#include "hardware/i2c.h"
#include <stdio.h>

#define BME280_I2C_ADDR 0x76

static bool initialized = false;

void rp2040_i2c_read(uint8_t* buffer, uint16_t length)
{
    i2c_read_timeout_us(i2c1, BME280_I2C_ADDR, buffer, length, false, 100000);
}

void rp2040_i2c_write(uint8_t* data, uint16_t size)
{
    i2c_write_timeout_us(i2c1, BME280_I2C_ADDR, data, size, false, 100000);
}

bool bme280_wrapper_init(void)
{
    i2c_init(i2c1, 100000);
    
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    gpio_pull_up(14);
    gpio_pull_up(15);
    bme280_init(rp2040_i2c_read, rp2040_i2c_write);
    
    initialized = true;
    return true;
}

bme280_data_t bme280_wrapper_read(void)
{
    bme280_data_t data = {0};
    
    if (!initialized) return data;
    
    data.temperature = bme280_read_temperature_celsius();
    data.pressure = bme280_read_pressure_hpa();
    data.humidity = bme280_read_humidity_percent();
    
    return data;
}

void bme280_wrapper_get_raw(int32_t* temp_raw, int32_t* pres_raw, int32_t* hum_raw)
{
    if (!initialized) return;
    
    *temp_raw = bme280_read_temp_raw_20bit();
    *pres_raw = bme280_read_pres_raw_20bit();
    *hum_raw = bme280_read_hum_raw_16bit();
}