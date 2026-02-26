#include "protocol-task.h"
#include <stdio.h>
#include <string.h>

static api_t* api = {0};
static int commands_count = 0;

// П 12
void protocol_task_init(api_t* device_api)
{
    api = device_api;
    commands_count = 0;
    
    // колво команд 
    while (api[commands_count].command_name != NULL)
    {
        commands_count++;
    }
}

// П 13
void protocol_task_handle(char* command_string)
{
    
    if (!command_string)
    {
        return; 
    }
    
    // Делим строку на команду и аргументы
    const char* command_name = command_string;
    const char* command_args = NULL;
    
    char* space_symbol = strchr(command_string, ' ');
    if (space_symbol)
    {
        *space_symbol = '\0';
        command_args = space_symbol + 1;
    }
    else
    {
        command_args = "";
    }
    
    // Вывод найденных имени команды и ее аргументов
    printf("command: '%s', args: '%s'\n", command_name, command_args);
    
    // Поиск команды в массиве
    for (int i = 0; i < commands_count; i++)
    {
        // Сравниваем имя команды
        if (strcmp(command_name, api[i].command_name) == 0)
        {
            // Команда найдена - вызываем колбэк
            api[i].command_callback(command_args);
            return;
        }
    }
    
    // Команда не найдена
    printf("Error: unknown command '%s'\n", command_name);
}