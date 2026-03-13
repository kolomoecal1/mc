#pragma once

#include <stdint.h>

// П 4
typedef void(*command_callback_t)(const char* args);

// П 5
typedef struct
{
    const char* command_name;        // имя команды
    command_callback_t command_callback;  // колбэк команды
    const char* command_help;         // описание команды
} api_t;

// П 6
void protocol_task_init(api_t* device_api);

// П 7
void protocol_task_handle(char* command_string);