#pragma once

#include <stdint.h>
#include "../tasks/measurement_task.h"  

typedef enum {
    SCREEN_MAIN,
    SCREEN_GRAPH_T,
    SCREEN_GRAPH_P,
    SCREEN_GRAPH_H,
    SCREEN_SETTINGS,
    SCREEN_COUNT
} gui_screen_t;

void gui_init(void);
void gui_set_screen(gui_screen_t screen);
gui_screen_t gui_get_screen(void);
void gui_next_screen(void);
void gui_draw_screen(const measurement_history_t* history);
void gui_handle_button(void);