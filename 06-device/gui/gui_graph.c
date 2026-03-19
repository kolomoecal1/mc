#include "gui_graph.h"
#include "../display/display_wrapper.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define GRAPH_DEFAULT_BG    0x1082      // Темно-серый
#define GRAPH_DEFAULT_GRID   0x3186      // Серый
#define GRAPH_DEFAULT_LINE   0xFFE0      // Желтый

void gui_graph_init(graph_config_t* config)
{
    if (!config) return;
    
    config->x = 10;
    config->y = 30;
    config->width = 300;
    config->height = 170;
    config->min_value = 0.0f;
    config->max_value = 100.0f;
    config->bg_color = GRAPH_DEFAULT_BG;
    config->grid_color = GRAPH_DEFAULT_GRID;
    config->line_color = GRAPH_DEFAULT_LINE;
    config->show_grid = true;
    config->show_labels = true;
}

static float get_data_min(const float* data, uint16_t count)
{
    if (count == 0) return 0;
    
    float min = data[0];
    for (uint16_t i = 1; i < count; i++)
    {
        if (data[i] < min) min = data[i];
    }
    return min;
}

static float get_data_max(const float* data, uint16_t count)
{
    if (count == 0) return 100;
    
    float max = data[0];
    for (uint16_t i = 1; i < count; i++)
    {
        if (data[i] > max) max = data[i];
    }
    return max;
}

static void draw_grid(const graph_config_t* config)
{
    if (!config || !config->show_grid) return;
    
    for (int i = 0; i <= 4; i++)
    {
        uint16_t yy = config->y + (i * config->height / 4);
        ili9341_draw_line(display_get_context(), 
                         config->x, yy, 
                         config->x + config->width, yy, 
                         config->grid_color);
    }
    
    for (int i = 0; i <= 6; i++)
    {
        uint16_t xx = config->x + (i * config->width / 6);
        ili9341_draw_line(display_get_context(), 
                         xx, config->y, 
                         xx, config->y + config->height, 
                         config->grid_color);
    }
}

static void draw_labels(const graph_config_t* config, 
                        float min_val, float max_val)
{
    if (!config || !config->show_labels) return;
    
    char buffer[16];
    
    // Подписи по вертикали (значения)
    for (int i = 0; i <= 4; i++)
    {
        float value = max_val - (i * (max_val - min_val) / 4);
        snprintf(buffer, sizeof(buffer), "%.0f", value);
        
        uint16_t yy = config->y + (i * config->height / 4) - 5;
        ili9341_draw_text(display_get_context(),
                         config->x - 40, yy,
                         buffer, &jetbrains_font,
                         config->grid_color, config->bg_color);
    }
    
    // Подписи по горизонтали (время)
    ili9341_draw_text(display_get_context(),
                     config->x, config->y + config->height + 5,
                     "60s ago", &jetbrains_font,
                     config->grid_color, config->bg_color);
    
    ili9341_draw_text(display_get_context(),
                     config->x + config->width - 30, 
                     config->y + config->height + 5,
                     "now", &jetbrains_font,
                     config->grid_color, config->bg_color);
}

void gui_graph_draw(const graph_config_t* config, 
                    const float* data, 
                    uint16_t data_count)
{
    if (!config || !data || data_count < 2) return;
    
    // Рисуем фон
    ili9341_draw_filled_rect(display_get_context(),
                            config->x, config->y,
                            config->width, config->height,
                            config->bg_color);
    
    draw_grid(config);
    
    float x_step = (float)config->width / (data_count - 1);
    float y_range = config->max_value - config->min_value;
    
    for (uint16_t i = 0; i < data_count - 1; i++)
    {
        float val1 = (data[i] - config->min_value) / y_range;
        float val2 = (data[i + 1] - config->min_value) / y_range;
        
        if (val1 < 0) val1 = 0;
        if (val1 > 1) val1 = 1;
        if (val2 < 0) val2 = 0;
        if (val2 > 1) val2 = 1;
        
        uint16_t x1 = config->x + (uint16_t)(i * x_step);
        uint16_t x2 = config->x + (uint16_t)((i + 1) * x_step);
        uint16_t y1 = config->y + config->height - (uint16_t)(val1 * config->height);
        uint16_t y2 = config->y + config->height - (uint16_t)(val2 * config->height);
        ili9341_draw_line(display_get_context(), x1, y1, x2, y2, 
                         config->line_color);
    }
    
    ili9341_draw_rect(display_get_context(),
                     config->x, config->y,
                     config->width, config->height,
                     COLOR_WHITE);
    draw_labels(config, config->min_value, config->max_value);
}

void gui_graph_draw_auto(const graph_config_t* config,
                         const float* data,
                         uint16_t data_count)
{
    if (!config || !data || data_count < 2) return;
    float min_val = get_data_min(data, data_count);
    float max_val = get_data_max(data, data_count);
    float range = max_val - min_val;
    float padding = range * 0.1f;
    if (padding < 0.1f) padding = 0.1f;
    
    graph_config_t auto_config = *config;
    auto_config.min_value = min_val - padding;
    auto_config.max_value = max_val + padding;
    
    gui_graph_draw(&auto_config, data, data_count);
}

void gui_graph_add_label(const graph_config_t* config,
                         const char* text,
                         uint16_t x, uint16_t y,
                         uint16_t color)
{
    if (!config || !text) return;
    
    ili9341_draw_text(display_get_context(),
                     config->x + x, config->y + y,
                     text, &jetbrains_font,
                     color, config->bg_color);
}

void gui_graph_clear(const graph_config_t* config)
{
    if (!config) return;
    
    ili9341_draw_filled_rect(display_get_context(),
                            config->x, config->y,
                            config->width, config->height,
                            config->bg_color);
}