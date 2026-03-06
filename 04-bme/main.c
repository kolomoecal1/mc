#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "stdio-task/stdio-task.h"
#include "led-task/led-task.h"
#include "protocol-task.h"
#include "bme280-driver.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"
#define BME280_I2C_ADDR 0x76

void rp2040_i2c_read(uint8_t* buffer, uint16_t length)
{
    i2c_read_timeout_us(i2c1, BME280_I2C_ADDR, buffer, length, false, 100000);
}

void rp2040_i2c_write(uint8_t* data, uint16_t size)
{
    i2c_write_timeout_us(i2c1, BME280_I2C_ADDR, data, size, false, 100000);
}

void led_on_callback(const char* args)
{
    led_task_state_set(LED_STATE_ON);
    printf("led enable done\n");
}

void led_off_callback(const char* args)
{
    led_task_state_set(LED_STATE_OFF);
    printf("led disable done\n");
}

void led_blink_callback(const char* args)
{
    led_task_state_set(LED_STATE_BLINK);
    printf("led blink mode enabled\n");
}

void led_blink_set_period_ms_callback(const char* args)
{
    uint period_ms = 0;
    sscanf(args, "%u", &period_ms);
    
    if (period_ms == 0)
    {
        printf("Error: period must be > 0 ms\n");
        return;
    }
    
    led_task_set_blink_period_ms(period_ms);
    printf("LED blink period set to %u ms\n", period_ms);
}

void read_reg_callback(const char* args)
{
    uint addr = 0, n = 0;
    sscanf(args, "%x %x", &addr, &n);
    
    if (addr > 0xFF || n > 0xFF || addr + n > 0x100)
    {
        printf("Error: invalid address or count\n");
        return;
    }
    
    uint8_t buffer[256] = {0};
    bme280_read_regs((uint8_t)addr, buffer, (uint8_t)n);
    
    printf("BME280 register dump:\n");
    for (int i = 0; i < n; i++)
    {
        printf("  bme280 register [0x%02X] = 0x%02X\n", addr + i, buffer[i]);
    }
}

void write_reg_callback(const char* args)
{
    uint addr = 0, value = 0;
    sscanf(args, "%x %x", &addr, &value);
    
    if (addr > 0xFF || value > 0xFF)
    {
        printf("Error: address or value must be <= 0xFF\n");
        return;
    }
    
    bme280_write_reg((uint8_t)addr, (uint8_t)value);
    printf("Written 0x%02X to register 0x%02X\n", value, addr);
}

void temp_raw_callback(const char* args)
{
    uint16_t temp = bme280_read_temp_raw();
    printf("Temperature raw: %u\n", temp);
}

void pres_raw_callback(const char* args)
{
    uint16_t pres = bme280_read_pres_raw();
    printf("Pressure raw: %u\n", pres);
}

void hum_raw_callback(const char* args)
{
    uint16_t hum = bme280_read_hum_raw();
    printf("Humidity raw: %u\n", hum);
}

void temp_callback(const char* args)
{
    float temp = bme280_read_temperature_celsius();
    printf("%.2f C\n", temp);
}

void pres_callback(const char* args)
{
    float pres = bme280_read_pressure_hpa();
    printf("%.2f hPa\n", pres);
}

void hum_callback(const char* args)
{
    float hum = bme280_read_humidity_percent();
    printf("%.2f %%\n", hum);
}

void version_callback(const char* args)
{
    printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

api_t device_api[] =
{
    {"version", version_callback, "get device name and firmware version"},
    {"on", led_on_callback, "turn LED on"},
    {"off", led_off_callback, "turn LED off"},
    {"blink", led_blink_callback, "make LED blink"},
    {"period", led_blink_set_period_ms_callback, "set blink period in ms"},
    {"read_reg", read_reg_callback, "read BME280 registers"},
    {"write_reg", write_reg_callback, "write BME280 register"},
    {"temp_raw", temp_raw_callback, "read raw temperature value"},
    {"pres_raw", pres_raw_callback, "read raw pressure value"},
    {"hum_raw", hum_raw_callback, "read raw humidity value"},
    {"temp", temp_callback, "read temperature in °C"},
    {"pres", pres_callback, "read pressure in hPa"},
    {"hum", hum_callback, "read humidity in %"},
    {NULL, NULL, NULL},
};

int main() {
    stdio_init_all();
    
    led_task_init();
    stdio_task_init();
    
    i2c_init(i2c1, 100000);
    
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    gpio_pull_up(14);
    gpio_pull_up(15);
    
    bme280_init(rp2040_i2c_read, rp2040_i2c_write);
    
    protocol_task_init(device_api);
    
    sleep_ms(2000);
    
    printf("\n========================================\n");
    printf("    04-bme: BME280 SI Units             \n");
    printf("========================================\n");
    printf("Commands:\n");
    printf("  version           - get device info\n");
    printf("  on/off/blink      - LED control\n");
    printf("  period <ms>       - set blink period\n");
    printf("  read_reg <addr> <count> - read registers\n");
    printf("  write_reg <addr> <val>  - write register\n");
    printf("  temp_raw/pres_raw/hum_raw - raw values\n");
    printf("  temp/pres/hum      - values in SI units\n");
    printf("\nType command and press ENTER:\n");
    printf("----------------------------------------\n\n");
    printf("> ");
    
    while (1) {
        char* command = stdio_task_handle();
        
        if (command != NULL) {
            protocol_task_handle(command);
            printf("\n> ");
        }
        
        led_task_handler();
    }
    
    return 0;
}