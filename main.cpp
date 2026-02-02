#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ht16k33.h"

// I2C pins
#define I2C_SDA_PIN 14 
#define I2C_SCL_PIN 15

int main()
{
    stdio_init_all();
    
    // Initialize I2C1 (pins 14/15)
    i2c_init(i2c1, 100 * 1000); // 100kHz
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    
    // Initialize display with i2c1 instance
    ht16k33::HT16K33 display(i2c1);
    display.begin();

    while (true) {
        display.displayDigit(0, 0xB, false); 
        display.displayDigit(1, 0xA, false); 
        display.displayDigit(2, 0xA, false); 
        display.displayDigit(3, 0xD, false); 
        display.writeDisplay();
        printf("0xBAAD\n");
        sleep_ms(1000);
        
        display.displayDigit(0, 0xF, true); 
        display.displayDigit(1, 0x0, false); 
        display.displayDigit(2, 0x0, false);
        display.displayDigit(3, 0xD, false); 
        display.writeDisplay();
        printf("0xF00D\n");
        sleep_ms(1000);
    }
}
