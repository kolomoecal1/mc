#include "stdio-task/stdio-task.h"
#include "protocol-task.h"
#include "pico/stdlib.h"
#include "stdio.h"
#include "stdlib.h"
#include "led-task/led-task.h"
#include "hardware/i2c.h"
#include "bme280-driver.h"
#include "hardware/spi.h"
#include "ili9341-driver.h"
#include "ili9341-display.h"
#include "ili9341-font.h"
#include "font-jetbrains.h"

#define DEVICE_NAME "BME280 Monitor"
#define DEVICE_VRSN "v1.0"

#define ILI9341_PIN_MISO 4
#define ILI9341_PIN_CS 10
#define ILI9341_PIN_SCK 6
#define ILI9341_PIN_MOSI 7
#define ILI9341_PIN_DC 8
#define ILI9341_PIN_RESET 9

#define GRAPH_START_X 10
#define GRAPH_START_Y 15
#define GRAPH_WIDTH 300
#define GRAPH_HEIGHT 70
#define GRAPH_SPACING 75
#define COLOR_GREY 0x8410

#define MAX_POINTS 60
#define PIXELS_PER_POINT 5
#define BME280_TELEMETRY_PERIOD_US 3000000  

typedef struct {
    float values[MAX_POINTS];
    int head;
    int count;
    float min_val;
    float max_val;
} history_t;

static history_t temp_history = {0};
static history_t pres_history = {0};
static history_t hum_history = {0};

static float last_temp = 0;
static float last_pres = 0;
static float last_hum = 0;
static bool telemetry_active = true;

static ili9341_display_t ili9341_display = {0};

void rp2040_spi_write(const uint8_t* data, uint32_t size) {
    spi_write_blocking(spi0, data, size);
}

void rp2040_spi_read(uint8_t* buffer, uint32_t length) {
    spi_read_blocking(spi0, 0, buffer, length);
}

void rp2040_gpio_cs_write(bool level) {
    gpio_put(ILI9341_PIN_CS, level);
}

void rp2040_gpio_dc_write(bool level) {
    gpio_put(ILI9341_PIN_DC, level);
}

void rp2040_gpio_reset_write(bool level) {
    gpio_put(ILI9341_PIN_RESET, level);
}

void rp2040_delay_ms(uint32_t ms) {
    sleep_ms(ms);
}

void rp2040_i2c_read(uint8_t* buffer, uint16_t length) {
    i2c_read_timeout_us(i2c1, 0x76, buffer, length, false, 100000);
}

void rp2040_i2c_write(uint8_t* data, uint16_t size) {
    i2c_write_timeout_us(i2c1, 0x76, data, size, false, 100000);
}

void init_history(history_t* hist, float min, float max) {
    hist->head = 0;
    hist->count = 0;
    hist->min_val = min;
    hist->max_val = max;
    for (int i = 0; i < MAX_POINTS; i++) {
        hist->values[i] = 0;
    }
}

void add_to_history(history_t* hist, float value) {
    hist->values[hist->head] = value;
    hist->head = (hist->head + 1) % MAX_POINTS;
    if (hist->count < MAX_POINTS) {
        hist->count++;
    }
}

float get_from_history(history_t* hist, int index) {
    if (index >= hist->count) return 0;
    int pos = (hist->head - hist->count + index + MAX_POINTS) % MAX_POINTS;
    return hist->values[pos];
}

int value_to_y(float value, history_t* hist, int y_start, int height) {
    float range = hist->max_val - hist->min_val;
    if (range == 0) return y_start + height/2;
    float normalized = (value - hist->min_val) / range;
    return y_start + height - (int)(normalized * height);
}

void ili9341_draw_filled_circle(ili9341_display_t* disp, int16_t x0, int16_t y0, int16_t radius, uint16_t color) {
    int16_t x = radius;
    int16_t y = 0;
    int16_t err = 0;
    
    while (x >= y) {
        for (int i = -x; i <= x; i++) {
            if (y0 - y >= 0 && y0 - y < 240 && x0 + i >= 0 && x0 + i < 320)
                ili9341_draw_pixel(disp, x0 + i, y0 - y, color);
            if (y0 + y >= 0 && y0 + y < 240 && x0 + i >= 0 && x0 + i < 320)
                ili9341_draw_pixel(disp, x0 + i, y0 + y, color);
        }
        for (int i = -y; i <= y; i++) {
            if (y0 - x >= 0 && y0 - x < 240 && x0 + i >= 0 && x0 + i < 320)
                ili9341_draw_pixel(disp, x0 + i, y0 - x, color);
            if (y0 + x >= 0 && y0 + x < 240 && x0 + i >= 0 && x0 + i < 320)
                ili9341_draw_pixel(disp, x0 + i, y0 + x, color);
        }
        
        y++;
        err += 1 + 2*y;
        if (2*(err - x) + 1 > 0) {
            x--;
            err += 1 - 2*x;
        }
    }
}

void draw_graph_axes(history_t* hist, const char* title, uint16_t title_color, 
                     float current_value, int y_start) {
    ili9341_draw_rect(&ili9341_display, GRAPH_START_X, y_start, 
                      GRAPH_WIDTH, GRAPH_HEIGHT, COLOR_WHITE);
    
    char title_text[48];
    sprintf(title_text, "%s: %.1f", title, current_value);
    ili9341_draw_text(&ili9341_display, GRAPH_START_X + 5, y_start - 12, 
                      title_text, &jetbrains_font, title_color, COLOR_BLACK);
    
    char min_text[12];
    char max_text[12];
    sprintf(min_text, "%.0f", hist->min_val);
    sprintf(max_text, "%.0f", hist->max_val);
    
    ili9341_draw_text(&ili9341_display, GRAPH_START_X + GRAPH_WIDTH - 30, y_start + 2, 
                      max_text, &jetbrains_font, COLOR_WHITE, COLOR_BLACK);
    ili9341_draw_text(&ili9341_display, GRAPH_START_X + GRAPH_WIDTH - 30, y_start + GRAPH_HEIGHT - 12, 
                      min_text, &jetbrains_font, COLOR_WHITE, COLOR_BLACK);
    
    for (int i = 1; i < 4; i++) {
        int y = y_start + i * GRAPH_HEIGHT / 4;
        for (int x = GRAPH_START_X; x < GRAPH_START_X + GRAPH_WIDTH; x += 4) {
            ili9341_draw_pixel(&ili9341_display, x, y, COLOR_GREY);
        }
    }
}

void draw_graph_data(history_t* hist, uint16_t color, int y_start) {
    if (hist->count < 2) {
        if (hist->count == 1) {
            int x = GRAPH_START_X;
            int y = value_to_y(get_from_history(hist, 0), hist, y_start, GRAPH_HEIGHT);
            ili9341_draw_filled_circle(&ili9341_display, x, y, 2, color);
        }
        return;
    }
    
    int prev_x = GRAPH_START_X;
    int prev_y = value_to_y(get_from_history(hist, 0), hist, y_start, GRAPH_HEIGHT);
    
    for (int i = 1; i < hist->count; i++) {
        int x = GRAPH_START_X + i * PIXELS_PER_POINT;
        int y = value_to_y(get_from_history(hist, i), hist, y_start, GRAPH_HEIGHT);
        
        ili9341_draw_line(&ili9341_display, prev_x, prev_y, x, y, color);
        ili9341_draw_filled_circle(&ili9341_display, x, y, 2, color);
        
        prev_x = x;
        prev_y = y;
    }
}

void init_all_graphs() {
    init_history(&temp_history, 15.0f, 35.0f);
    init_history(&pres_history, 98.0f, 103.0f);
    init_history(&hum_history, 10.0f, 80.0f);
}

void draw_all_graphs() {
    ili9341_draw_filled_rect(&ili9341_display, GRAPH_START_X-2, GRAPH_START_Y-15, 
                             GRAPH_WIDTH+4, GRAPH_HEIGHT*3 + GRAPH_SPACING*2 + 20, COLOR_BLACK);
    
    draw_graph_axes(&temp_history, "Temperature", COLOR_RED, last_temp, GRAPH_START_Y);
    draw_graph_axes(&pres_history, "Pressure", COLOR_GREEN, last_pres / 1000.0f, 
                    GRAPH_START_Y + GRAPH_SPACING);
    draw_graph_axes(&hum_history, "Humidity", COLOR_BLUE, last_hum, 
                    GRAPH_START_Y + GRAPH_SPACING * 2);
    
    draw_graph_data(&temp_history, COLOR_RED, GRAPH_START_Y);
    draw_graph_data(&pres_history, COLOR_GREEN, GRAPH_START_Y + GRAPH_SPACING);
    draw_graph_data(&hum_history, COLOR_BLUE, GRAPH_START_Y + GRAPH_SPACING * 2);
}

void update_graphs_with_measurement(float temp, float pres, float hum) {
    add_to_history(&temp_history, temp);
    add_to_history(&pres_history, pres / 1000.0f);
    add_to_history(&hum_history, hum);
    
    last_temp = temp;
    last_pres = pres;
    last_hum = hum;
    
    draw_all_graphs();
}

void bme280_telemetry_handler() {
    static uint64_t last_measure_us = 0;
    uint64_t now = time_us_64();
    
    if (!telemetry_active) return;
    
    if (now >= last_measure_us + BME280_TELEMETRY_PERIOD_US) {
        last_measure_us = now;
        
        float temp = bme280_read_temperature();
        float pres = bme280_read_pressure();
        float hum = bme280_read_humidity();
        
        printf("%.2f %.2f %.2f\n", temp, pres, hum);
        update_graphs_with_measurement(temp, pres, hum);
    }
}

void help_callback(const char* args) {
    printf("  help           - show this help\n");
    printf("  version        - show firmware version\n");
    printf("  on/off/blink   - control onboard LED\n");
    printf("  temp/pres/hum  - single measurement\n");
    printf("  start/stop     - start/stop telemetry\n");
    printf("  clear          - clear graphs\n");
}

void version_callback(const char* args) {
    printf("%s %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void led_on_callback(const char* args) {
    led_task_state_set(LED_STATE_ON);
    printf("LED ON\n");
}

void led_off_callback(const char* args) {
    led_task_state_set(LED_STATE_OFF);
    printf("LED OFF\n");
}

void led_blink_callback(const char* args) {
    led_task_state_set(LED_STATE_BLINK);
    printf("LED blinking\n");
}

void temp_callback(const char* args) {
    float temp = bme280_read_temperature();
    printf("Temperature: %.2f °C\n", temp);
}

void pres_callback(const char* args) {
    float pres = bme280_read_pressure();
    printf("Pressure: %.2f Pa (%.2f hPa)\n", pres, pres/100.0f);
}

void hum_callback(const char* args) {
    float hum = bme280_read_humidity();
    printf("Humidity: %.2f %%\n", hum);
}

void start_callback(const char* args) {
    telemetry_active = true;
    printf("Telemetry started\n");
}

void stop_callback(const char* args) {
    telemetry_active = false;
    printf("Telemetry stopped\n");
}

void clear_callback(const char* args) {
    init_all_graphs();
    draw_all_graphs();
    printf("Graphs cleared\n");
}

api_t device_api[] = {
    {"help", help_callback, "show help"},
    {"version", version_callback, "show version"},
    {"on", led_on_callback, "turn LED on"},
    {"off", led_off_callback, "turn LED off"},
    {"blink", led_blink_callback, "make LED blink"},
    {"temp", temp_callback, "read temperature"},
    {"pres", pres_callback, "read pressure"},
    {"hum", hum_callback, "read humidity"},
    {"start", start_callback, "start telemetry"},
    {"stop", stop_callback, "stop telemetry"},
    {"clear", clear_callback, "clear graphs"},
    {NULL, NULL, NULL},
};

int main() {
    stdio_init_all();
    stdio_task_init();
    led_task_init();
    protocol_task_init(device_api);
    
    i2c_init(i2c1, 100000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    bme280_init(rp2040_i2c_read, rp2040_i2c_write);
    
    spi_init(spi0, 62500000);
    gpio_set_function(ILI9341_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_SCK, GPIO_FUNC_SPI);
    
    gpio_init(ILI9341_PIN_CS);
    gpio_set_dir(ILI9341_PIN_CS, GPIO_OUT);
    gpio_init(ILI9341_PIN_DC);
    gpio_set_dir(ILI9341_PIN_DC, GPIO_OUT);
    gpio_init(ILI9341_PIN_RESET);
    gpio_set_dir(ILI9341_PIN_RESET, GPIO_OUT);
    
    gpio_put(ILI9341_PIN_CS, 1);
    gpio_put(ILI9341_PIN_DC, 0);
    gpio_put(ILI9341_PIN_RESET, 1);
    sleep_ms(100);
    
    ili9341_hal_t ili9341_hal = {0};
    ili9341_hal.spi_write = rp2040_spi_write;
    ili9341_hal.spi_read = rp2040_spi_read;
    ili9341_hal.gpio_cs_write = rp2040_gpio_cs_write;
    ili9341_hal.gpio_dc_write = rp2040_gpio_dc_write;
    ili9341_hal.gpio_reset_write = rp2040_gpio_reset_write;
    ili9341_hal.delay_ms = rp2040_delay_ms;
    
    ili9341_init(&ili9341_display, &ili9341_hal);
    ili9341_set_rotation(&ili9341_display, ILI9341_ROTATION_90);
    
    ili9341_fill_screen(&ili9341_display, COLOR_BLACK);
    init_all_graphs();
    draw_all_graphs();
    
    printf("\nBME280 Monitor Ready\n");
    printf("Type 'help' for commands\n\n");
    
    while (true) {
        char* command_string = stdio_task_handle();
        protocol_task_handle(command_string);
        led_task_handle();
        bme280_telemetry_handler();
    }
}