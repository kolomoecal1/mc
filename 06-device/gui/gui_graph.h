#pragma once

#include <stdint.h>
#include <stdbool.h>
typedef struct {
    uint16_t x;
    uint16_t y;
} graph_point_t;
typedef struct {
    uint16_t x;           // X координата левого верхнего угла
    uint16_t y;           // Y координата левого верхнего угла
    uint16_t width;       // Ширина области графика
    uint16_t height;      // Высота области графика
    float min_value;      // Минимальное значение на графике
    float max_value;      // Максимальное значение на графике
    uint16_t bg_color;    // Цвет фона
    uint16_t grid_color;  // Цвет сетки
    uint16_t line_color;  // Цвет линии графика
    bool show_grid;       // Показывать сетку?
    bool show_labels;     // Показывать подписи?
} graph_config_t;
void gui_graph_init(graph_config_t* config);
void gui_graph_draw(const graph_config_t* config, 
                    const float* data, 
                    uint16_t data_count);
void gui_graph_draw_auto(const graph_config_t* config,
                         const float* data,
                         uint16_t data_count);
void gui_graph_add_label(const graph_config_t* config,
                         const char* text,
                         uint16_t x, uint16_t y,
                         uint16_t color);
void gui_graph_clear(const graph_config_t* config);