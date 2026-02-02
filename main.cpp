#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pins.h"
#include "ht16k33.h"



int main()
{
    stdio_init_all();
    initializePins();
    
    // Initialize display with i2c1 instance
    ht16k33::HT16K33 display(i2c1);
    display.begin();

    // Test the display
    display.testDisplay();

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

        display.displayOutlineChase();

    }
}
