#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "stdio-task/stdio-task.h"
#include "led-task/led-task.h"
#include "protocol-task.h"
#include "ili9341-display.h"
#include "ili9341-font.h"

#define ILI9341_PIN_MISO 4
#define ILI9341_PIN_CS   10
#define ILI9341_PIN_SCK   6
#define ILI9341_PIN_MOSI  7
#define ILI9341_PIN_DC    8
#define ILI9341_PIN_RESET 9
#define LED_PIN 25

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

static ili9341_display_t ili9341_display = {0};

void rp2040_spi_write(const uint8_t *data, uint32_t size)
{
    spi_write_blocking(spi0, data, size);
}

void rp2040_spi_read(uint8_t *buffer, uint32_t length)
{
    spi_read_blocking(spi0, 0, buffer, length);
}

void rp2040_gpio_cs_write(bool level)
{
    gpio_put(ILI9341_PIN_CS, level);
}

void rp2040_gpio_dc_write(bool level)
{
    gpio_put(ILI9341_PIN_DC, level);
}

void rp2040_gpio_reset_write(bool level)
{
    gpio_put(ILI9341_PIN_RESET, level);
}

void rp2040_delay_ms(uint32_t ms)
{
    sleep_ms(ms);
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

void version_callback(const char* args)
{
    printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void disp_screen_callback(const char* args)
{
    uint32_t c = 0;
    int result = sscanf(args, "%x", &c);
    
    uint16_t color = COLOR_BLACK;
    
    if (result == 1)
    {
        color = RGB888_2_RGB565(c);
        printf("Setting screen to color 0x%06X\n", c);
    }
    else
    {
        printf("Setting screen to default color (BLACK)\n");
    }
    
    ili9341_fill_screen(&ili9341_display, color);
}

void disp_px_callback(const char* args)
{
    uint32_t x = 0, y = 0, c = 0;
    
    int result = sscanf(args, "%u %u %x", &x, &y, &c);
    
    if (result < 3)
    {
        printf("Error: disp_px requires 3 arguments: x y color (hex)\n");
        printf("Example: disp_px 100 150 FF0000\n");
        return;
    }
    
    if (x >= ili9341_display.width || y >= ili9341_display.height)
    {
        printf("Error: coordinates out of bounds (max %ux%u)\n", 
               ili9341_display.width, ili9341_display.height);
        return;
    }
    
    uint16_t color = RGB888_2_RGB565(c);
    ili9341_draw_pixel(&ili9341_display, (uint16_t)x, (uint16_t)y, color);
    
    printf("Pixel drawn at (%u, %u) with color 0x%06X\n", x, y, c);
}

void disp_line_callback(const char* args)
{
    uint32_t x0 = 0, y0 = 0, x1 = 0, y1 = 0, c = 0;
    
    int result = sscanf(args, "%u %u %u %u %x", &x0, &y0, &x1, &y1, &c);
    
    if (result < 5)
    {
        printf("Error: disp_line requires 5 arguments: x0 y0 x1 y1 color (hex)\n");
        printf("Example: disp_line 10 10 200 100 FF0000\n");
        return;
    }
    
    if (x0 >= ili9341_display.width || y0 >= ili9341_display.height ||
        x1 >= ili9341_display.width || y1 >= ili9341_display.height)
    {
        printf("Error: coordinates out of bounds (max %ux%u)\n", 
               ili9341_display.width, ili9341_display.height);
        return;
    }
    
    uint16_t color = RGB888_2_RGB565(c);
    ili9341_draw_line(&ili9341_display, (uint16_t)x0, (uint16_t)y0, 
                     (uint16_t)x1, (uint16_t)y1, color);
    
    printf("Line drawn from (%u,%u) to (%u,%u) with color 0x%06X\n", 
           x0, y0, x1, y1, c);
}

void disp_rect_callback(const char* args)
{
    uint32_t x = 0, y = 0, w = 0, h = 0, c = 0;
    
    int result = sscanf(args, "%u %u %u %u %x", &x, &y, &w, &h, &c);
    
    if (result < 5)
    {
        printf("Error: disp_rect requires 5 arguments: x y width height color (hex)\n");
        printf("Example: disp_rect 50 50 100 80 FF0000\n");
        return;
    }
    
    if (x >= ili9341_display.width || y >= ili9341_display.height ||
        x + w > ili9341_display.width || y + h > ili9341_display.height)
    {
        printf("Error: rectangle out of bounds (max %ux%u)\n", 
               ili9341_display.width, ili9341_display.height);
        return;
    }
    
    uint16_t color = RGB888_2_RGB565(c);
    ili9341_draw_rect(&ili9341_display, (uint16_t)x, (uint16_t)y, 
                     (uint16_t)w, (uint16_t)h, color);
    
    printf("Rectangle drawn at (%u,%u) size %ux%u with color 0x%06X\n", 
           x, y, w, h, c);
}

void disp_frect_callback(const char* args)
{
    uint32_t x = 0, y = 0, w = 0, h = 0, c = 0;
    
    int result = sscanf(args, "%u %u %u %u %x", &x, &y, &w, &h, &c);
    
    if (result < 5)
    {
        printf("Error: disp_frect requires 5 arguments: x y width height color (hex)\n");
        printf("Example: disp_frect 50 50 100 80 00FF00\n");
        return;
    }
    
    if (x >= ili9341_display.width || y >= ili9341_display.height ||
        x + w > ili9341_display.width || y + h > ili9341_display.height)
    {
        printf("Error: filled rectangle out of bounds (max %ux%u)\n", 
               ili9341_display.width, ili9341_display.height);
        return;
    }
    
    uint16_t color = RGB888_2_RGB565(c);
    ili9341_draw_filled_rect(&ili9341_display, (uint16_t)x, (uint16_t)y, 
                            (uint16_t)w, (uint16_t)h, color);
    
    printf("Filled rectangle drawn at (%u,%u) size %ux%u with color 0x%06X\n", 
           x, y, w, h, c);
}

// Пункт 1-2: колбэк для команды disp_text
void disp_text_callback(const char* args)
{
    uint32_t x = 0, y = 0, text_color = 0, bg_color = 0;
    char text[64] = {0};
    
    int result = sscanf(args, "%u %u %x %x %63[^\n]", &x, &y, &text_color, &bg_color, text);
    
    if (result < 5)
    {
        printf("Error: disp_text requires 5 arguments: x y text_color bg_color \"text\"\n");
        printf("Example: disp_text 50 50 FFFFFF 000000 \"Hello World\"\n");
        printf("Note: text must be in quotes if it contains spaces\n");
        return;
    }
    
    if (x >= ili9341_display.width || y >= ili9341_display.height)
    {
        printf("Error: coordinates out of bounds (max %ux%u)\n", 
               ili9341_display.width, ili9341_display.height);
        return;
    }
    
    if (strlen(text) == 0)
    {
        printf("Error: text cannot be empty\n");
        return;
    }
    
    uint16_t fg_color = RGB888_2_RGB565(text_color);
    uint16_t bg = RGB888_2_RGB565(bg_color);
    
    ili9341_draw_text(&ili9341_display, (uint16_t)x, (uint16_t)y, text, 
                      &jetbrains_font, fg_color, bg);
    
    printf("Text \"%s\" drawn at (%u,%u) with color 0x%06X on background 0x%06X\n", 
           text, x, y, text_color, bg_color);
}

api_t device_api[] =
{
    {"version", version_callback, "get device name and firmware version"},
    {"on", led_on_callback, "turn LED on"},
    {"off", led_off_callback, "turn LED off"},
    {"blink", led_blink_callback, "make LED blink"},
    {"period", led_blink_set_period_ms_callback, "set blink period in ms"},
    {"disp_screen", disp_screen_callback, "fill screen with color (RGB888 hex)"},
    {"disp_px", disp_px_callback, "draw pixel: disp_px <x> <y> <color_hex>"},
    {"disp_line", disp_line_callback, "draw line: disp_line <x0> <y0> <x1> <y1> <color_hex>"},
    {"disp_rect", disp_rect_callback, "draw rectangle: disp_rect <x> <y> <w> <h> <color_hex>"},
    {"disp_frect", disp_frect_callback, "draw filled rect: disp_frect <x> <y> <w> <h> <color_hex>"},
    {"disp_text", disp_text_callback, "draw text: disp_text <x> <y> <color_hex> <bg_hex> \"text\""},
    {NULL, NULL, NULL},
};

int main()
{
    stdio_init_all();
    
    led_task_init();
    stdio_task_init();
    
    spi_init(spi0, 62500000);
    
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
    }
    else
    {
        printf("Failed to initialize ILI9341\n");
    }
    
    protocol_task_init(device_api);
    
    sleep_ms(1000);
    
    printf("\n========================================\n");
    printf("    05-display: Text Drawing Command     \n");
    printf("========================================\n");
    printf("Commands:\n");
    printf("  version                       - get device info\n");
    printf("  on/off/blink                  - LED control\n");
    printf("  period <ms>                    - set blink period\n");
    printf("  disp_screen [hex]              - fill screen with color\n");
    printf("  disp_px <x> <y> <hex>          - draw pixel\n");
    printf("  disp_line <x0> <y0> <x1> <y1> <hex> - draw line\n");
    printf("  disp_rect <x> <y> <w> <h> <hex>      - draw rectangle\n");
    printf("  disp_frect <x> <y> <w> <h> <hex>     - draw filled rectangle\n");
    printf("  disp_text <x> <y> <fg_hex> <bg_hex> \"text\" - draw text\n");
    printf("\nExamples:\n");
    printf("  disp_text 50 50 FFFFFF 000000 \"Hello World\"\n");
    printf("  disp_text 50 70 FF0000 000000 \"Red text\"\n");
    printf("  disp_text 50 90 000000 FFFFFF \"Black on white\"\n");
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