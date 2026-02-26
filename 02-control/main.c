#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

// Колбэк для включения светодиода
void led_on_callback(const char* args)
{
    led_task_state_set(LED_STATE_ON);
    printf("led enable done\n");
}

// Колбэк для выключения светодиода
void led_off_callback(const char* args)
{
    led_task_state_set(LED_STATE_OFF);
    printf("led disable done\n");
}

// Колбэк для мигания светодиода
void led_blink_callback(const char* args)
{
    led_task_state_set(LED_STATE_BLINK);
    printf("led blink mode enabled\n");
}

// КОЛБЭК ДЛЯ КОМАНДЫ PERIOD - УСТАНОВКА ПЕРИОДА МИГАНИЯ
void led_blink_set_period_ms_callback(const char* args)
{
    uint period_ms = 0;
    
    // Парсим аргумент (ожидаем число)
    sscanf(args, "%u", &period_ms);
    
    // Проверка на нулевое значение
    if (period_ms == 0)
    {
        printf("Error: period must be > 0 ms\n");
        return;
    }
    
    // Устанавливаем новый период
    led_task_set_blink_period_ms(period_ms);
    printf("LED blink period set to %u ms\n", period_ms);
}

// Колбэк для версии
void version_callback(const char* args)
{
    printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

// МАССИВ КОМАНД - ЗДЕСЬ ДОЛЖНА БЫТЬ КОМАНДА period!
api_t device_api[] =
{
    {"version", version_callback, "get device name and firmware version"},
    {"on", led_on_callback, "turn LED on"},
    {"off", led_off_callback, "turn LED off"},
    {"blink", led_blink_callback, "make LED blink"},
    {"period", led_blink_set_period_ms_callback, "set blink period in ms (e.g., 'period 200')"},  // <--- ЭТА СТРОКА
    {NULL, NULL, NULL},
};

int main() {
    stdio_init_all();
    
    led_task_init();
    stdio_task_init();
    protocol_task_init(device_api);
    
    sleep_ms(2000);
    
    printf("\n========================================\n");
    printf("    02-control: LED Control with Period  \n");
    printf("========================================\n");
    printf("Commands:\n");
    printf("  version           - get device info\n");
    printf("  on                - turn LED on\n");
    printf("  off               - turn LED off\n");
    printf("  blink             - make LED blink\n");
    printf("  period <ms>       - set blink period (e.g., 'period 200')\n");
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