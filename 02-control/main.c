#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "stdio-task/stdio-task.h"  

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"


#define LED_PIN 25

void process_command(const char* cmd) {
    if (strcmp(cmd, "e") == 0) {
        gpio_put(LED_PIN, 1);
        printf("led enable done\n");
    }
    else if (strcmp(cmd, "d") == 0) {
        gpio_put(LED_PIN, 0);
        printf("led disable done\n");
    }
    else if (strcmp(cmd, "v") == 0) {
        printf("device name: '%s', firmware version: %s\n", 
               DEVICE_NAME, DEVICE_VRSN);
    }
    else if (strlen(cmd) > 0) {
        printf("unknown command: %s\n", cmd);
    }
}

int main() {
    
    stdio_init_all();
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    stdio_task_init();
    
    sleep_ms(2000);
    
    printf("\n========================================\n");
    printf("    02-control: Non-blocking input      \n");
    printf("========================================\n");
    printf("Commands:\n");
    printf("  'e' - enable LED\n");
    printf("  'd' - disable LED\n");
    printf("  'v' - show device version\n");
    printf("\nType command and press ENTER:\n");
    printf("----------------------------------------\n\n");
    
    while (1) 
    {
        char* command = stdio_task_handle();
        
        if (command != NULL) {
            process_command(command);
        }
        
        
        tight_loop_contents();  
    }
    
    return 0;
}