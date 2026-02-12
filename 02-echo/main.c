#include <stdio.h>
#include "pico/stdlib.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"


#define LED_PIN 25

int main() {
    
    stdio_init_all();
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    sleep_ms(2000);
    
    printf("\n=== Echo Program with LED Control ===\n");
    printf("Commands:\n");
    printf("  'e' - enable LED\n");
    printf("  'd' - disable LED\n");
    printf("  'v' - show device version\n");
    printf("=====================================\n\n");
    
    while (1) {
       
        char symbol = getchar();
        
        printf("received char: %c [ASCII code: %d]\n", symbol, symbol);
        
        switch(symbol) {
            case 'e':  // Включение светодиода
                gpio_put(LED_PIN, 1);
                printf("led enable done\n");
                break;
                
            case 'd':  // Выключение светодиода
                gpio_put(LED_PIN, 0);
                printf("led disable done\n");
                break;
                
            case 'v':  // Вывод имени и версии устройства
                printf("device name: '%s', firmware version: %s\n", 
                       DEVICE_NAME, DEVICE_VRSN);
                break;
                
            default:
               
                break;
        }
    }
    
    return 0;
}