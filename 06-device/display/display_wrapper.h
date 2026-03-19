#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "ili9341-display.h"
#include "ili9341-font.h"
bool display_wrapper_init(void);

void display_clear(uint16_t color);
void display_text(uint16_t x, uint16_t y, const char* text, 
                  uint16_t fg, uint16_t bg);
void display_bar(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                 float value, float min, float max, uint16_t color);
void display_graph(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                   float* data, uint16_t count, float min, float max,
                   uint16_t color);
ili9341_display_t* display_get_context(void);