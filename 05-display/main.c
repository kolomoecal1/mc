#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

// Подключаем заголовочные файлы библиотек
#include "stdio-task/stdio-task.h"
#include "led-task/led-task.h"
#include "protocol-task.h"

// Подключаем заголовочный файл драйвера дисплея
#include "ili9341-display.h"  // <-- ВАЖНО: используем display, а не driver!

// Константы для пинов (проверьте соответствие вашему подключению!)
#define ILI9341_PIN_MISO 4
#define ILI9341_PIN_CS   10
#define ILI9341_PIN_SCK   6
#define ILI9341_PIN_MOSI  7
#define ILI9341_PIN_DC    8
#define ILI9341_PIN_RESET 9
// Пин светодиода на Pico
#define LED_PIN 25

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

// Статическая переменная контекста драйвера дисплея
static ili9341_display_t ili9341_display = {0};

// Платформозависимые функции SPI
void rp2040_spi_write(const uint8_t *data, uint32_t size)
{
    spi_write_blocking(spi0, data, size);
}

void rp2040_spi_read(uint8_t *buffer, uint32_t length)
{
    spi_read_blocking(spi0, 0, buffer, length);
}

// Функция CS (chip select)
void rp2040_gpio_cs_write(bool level)
{
    gpio_put(ILI9341_PIN_CS, level);
}

// Функция DC (data/command)
void rp2040_gpio_dc_write(bool level)
{
    gpio_put(ILI9341_PIN_DC, level);
}

// Функция RESET
void rp2040_gpio_reset_write(bool level)
{
    gpio_put(ILI9341_PIN_RESET, level);
}

// Функция задержки
void rp2040_delay_ms(uint32_t ms)
{
    sleep_ms(ms);
}

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

// Колбэк для установки периода мигания
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

// Колбэк для версии
void version_callback(const char* args)
{
    printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

// Массив команд
api_t device_api[] =
{
    {"version", version_callback, "get device name and firmware version"},
    {"on", led_on_callback, "turn LED on"},
    {"off", led_off_callback, "turn LED off"},
    {"blink", led_blink_callback, "make LED blink"},
    {"period", led_blink_set_period_ms_callback, "set blink period in ms"},
    {NULL, NULL, NULL},
};

int main()
{
    // Инициализация stdio для COM порта
    stdio_init_all();
    
    // Инициализация задач
    led_task_init();
    stdio_task_init();
    protocol_task_init(device_api);
    
    // Инициализация SPI на скорость 62.5 МГц
    spi_init(spi0, 62500000);
    
    // Настройка пинов SPI на альтернативную функцию
    gpio_set_function(ILI9341_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_SCK, GPIO_FUNC_SPI);
    
    gpio_init(ILI9341_PIN_CS);
    gpio_init(ILI9341_PIN_DC);
    gpio_init(ILI9341_PIN_RESET);
    
    gpio_set_dir(ILI9341_PIN_CS, GPIO_OUT);
    gpio_set_dir(ILI9341_PIN_DC, GPIO_OUT);
    gpio_set_dir(ILI9341_PIN_RESET, GPIO_OUT);
    
    gpio_put(ILI9341_PIN_CS, 1); 
    gpio_put(ILI9341_PIN_DC, 0);     
    gpio_put(ILI9341_PIN_RESET, 0);  
    
    ili9341_hal_t ili9341_hal = {0};
    ili9341_hal.spi_write = rp2040_spi_write;
    ili9341_hal.spi_read = rp2040_spi_read;
    ili9341_hal.gpio_cs_write = rp2040_gpio_cs_write;
    ili9341_hal.gpio_dc_write = rp2040_gpio_dc_write;
    ili9341_hal.gpio_reset_write = rp2040_gpio_reset_write;
    ili9341_hal.delay_ms = rp2040_delay_ms;
    
    if (ili9341_init(&ili9341_display, &ili9341_hal))
    {
        printf("ILI9341 initialized successfully\n");
        
        ili9341_set_rotation(&ili9341_display, ILI9341_ROTATION_90);
        
        ili9341_fill_screen(&ili9341_display, COLOR_BLACK);
        sleep_ms(300);
        
        ili9341_draw_filled_rect(&ili9341_display, 10, 10, 100, 60, COLOR_RED);
        ili9341_draw_filled_rect(&ili9341_display, 120, 10, 100, 60, COLOR_BLUE);
        ili9341_draw_filled_rect(&ili9341_display, 230, 10, 80, 60, COLOR_WHITE);
        
        ili9341_draw_rect(&ili9341_display, 10, 90, 300, 80, COLOR_WHITE);
        
        ili9341_draw_line(&ili9341_display, 0, 0, 319, 239, COLOR_YELLOW);
        ili9341_draw_line(&ili9341_display, 319, 0, 0, 239, COLOR_CYAN);
        ili9341_draw_text(&ili9341_display, 20, 100, "Potylicin", &jetbrains_font, COLOR_WHITE, COLOR_BLACK);
        ili9341_draw_text(&ili9341_display, 20, 116, "RP2040 / Pico SDK", &jetbrains_font, COLOR_YELLOW, COLOR_BLACK);
    }
    else
    {
        printf("Failed to initialize ILI9341\n");
    }
    
    sleep_ms(1000);
    
    printf("\n========================================\n");
    printf("    05-display: Graphics Test            \n");
    printf("========================================\n");
    printf("Commands:\n");
    printf("  version           - get device info\n");
    printf("  on                - turn LED on\n");
    printf("  off               - turn LED off\n");
    printf("  blink             - make LED blink\n");
    printf("  period <ms>       - set blink period\n");
    printf("\nType command and press ENTER:\n");
    printf("----------------------------------------\n\n");
    printf("> ");
    
    while (1)
    {
        char* command = stdio_task_handle();
        
        if (command != NULL)
        {
            protocol_task_handle(command);
            printf("\n> ");
        }
        
        led_task_handler();
    }
    
    return 0;
}