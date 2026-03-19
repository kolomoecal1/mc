#include "gui_screen.h"
#include "../tasks/measurement_task.h"  // теперь здесь определяем полный тип
#include "../sensors/bme280_wrapper.h"
#include "../display/display_wrapper.h"
#include <stdio.h>
#include <string.h>

static gui_screen_t current_screen = SCREEN_MAIN;

#define COLOR_TEMP RGB888_2_RGB565(0xFF5500)
#define COLOR_PRESS RGB888_2_RGB565(0x00AAFF)
#define COLOR_HUMID RGB888_2_RGB565(0x00FF55)

void gui_init(void)
{
    current_screen = SCREEN_MAIN;
}

void gui_set_screen(gui_screen_t screen)
{
    if (screen < SCREEN_COUNT)
    {
        current_screen = screen;
    }
}

gui_screen_t gui_get_screen(void)
{
    return current_screen;
}

void gui_next_screen(void)
{
    current_screen = (current_screen + 1) % SCREEN_COUNT;
}

static void draw_main_screen(const measurement_history_t* history)
{
    bme280_data_t last = measurement_get_last();
    char buffer[32];
    
    display_text(10, 5, "Atmosphere Monitor", COLOR_WHITE, COLOR_BLACK);
    display_text(10, 20, "------------------", COLOR_WHITE, COLOR_BLACK);
    
    snprintf(buffer, sizeof(buffer), "Temp: %5.1f C", last.temperature);
    display_text(10, 40, buffer, COLOR_TEMP, COLOR_BLACK);
    display_bar(200, 40, 100, 15, last.temperature, -10, 50, COLOR_TEMP);
    
    snprintf(buffer, sizeof(buffer), "Pres: %6.1f hPa", last.pressure);
    display_text(10, 65, buffer, COLOR_PRESS, COLOR_BLACK);
    display_bar(200, 65, 100, 15, last.pressure, 950, 1050, COLOR_PRESS);
    
    snprintf(buffer, sizeof(buffer), "Hum:  %5.1f %%", last.humidity);
    display_text(10, 90, buffer, COLOR_HUMID, COLOR_BLACK);
    display_bar(200, 90, 100, 15, last.humidity, 0, 100, COLOR_HUMID);
    
    snprintf(buffer, sizeof(buffer), "Period: %d ms", measurement_get_period());
    display_text(10, 115, buffer, COLOR_WHITE, COLOR_BLACK);
    
    display_text(10, 140, "Next screen: button", 0xAD55, COLOR_BLACK);
    display_text(10, 155, "Set period: 'period N'", 0xAD55, COLOR_BLACK);
}

static void draw_graph_screen(const measurement_history_t* history, 
                              const char* title, const float* data,
                              float min, float max, uint16_t color)
{
    char buffer[32];
    
    display_text(10, 5, title, COLOR_WHITE, COLOR_BLACK);
    
    if (history->count > 0) {
        snprintf(buffer, sizeof(buffer), "Now: %.1f", data[history->index - 1]);
        display_text(200, 5, buffer, color, COLOR_BLACK);
    }
    
    display_graph(10, 30, 300, 170, (float*)data, history->count, 
                  min, max, color);
    
    display_text(10, 210, "60s ago", 0xAD55, COLOR_BLACK);
    display_text(270, 210, "now", 0xAD55, COLOR_BLACK);
}

static void draw_settings_screen(const measurement_history_t* history)
{
    char buffer[32];
    
    display_text(10, 5, "Settings", COLOR_WHITE, COLOR_BLACK);
    display_text(10, 20, "--------", COLOR_WHITE, COLOR_BLACK);
    
    snprintf(buffer, sizeof(buffer), "Period: %d ms", measurement_get_period());
    display_text(10, 40, buffer, COLOR_WHITE, COLOR_BLACK);
    
    const char* status = measurement_is_enabled() ? "ON" : "OFF";
    snprintf(buffer, sizeof(buffer), "Measurements: %s", status);
    display_text(10, 60, buffer, COLOR_WHITE, COLOR_BLACK);
    
    snprintf(buffer, sizeof(buffer), "History: %d/%d", 
             history->count, 60);  //直接用数字代替HISTORY_SIZE
    display_text(10, 80, buffer, COLOR_WHITE, COLOR_BLACK);
    
    display_text(10, 110, "Commands:", 0xAD55, COLOR_BLACK);
    display_text(10, 125, "period 500", 0xAD55, COLOR_BLACK);
    display_text(10, 140, "measure on/off", 0xAD55, COLOR_BLACK);
    display_text(10, 155, "screen next", 0xAD55, COLOR_BLACK);
}

void gui_draw_screen(const measurement_history_t* history)
{
    if (!history) return;
    
    display_clear(COLOR_BLACK);
    
    switch (current_screen)
    {
        case SCREEN_MAIN:
            draw_main_screen(history);
            break;
            
        case SCREEN_GRAPH_T:
            draw_graph_screen(history, "Temperature History", 
                            history->temperature, -10, 50, COLOR_TEMP);
            break;
            
        case SCREEN_GRAPH_P:
            draw_graph_screen(history, "Pressure History", 
                            history->pressure, 950, 1050, COLOR_PRESS);
            break;
            
        case SCREEN_GRAPH_H:
            draw_graph_screen(history, "Humidity History", 
                            history->humidity, 0, 100, COLOR_HUMID);
            break;
            
        case SCREEN_SETTINGS:
            draw_settings_screen(history);
            break;
            
        default:
            break;
    }
}

void gui_handle_button(void)
{
    gui_next_screen();
}