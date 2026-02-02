// (C) Alan Ludwig 2026 All rights reserved.

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pins.h"


void initializePins() {
    
    // Initialize I2C0 (pins 12/13)
    i2c_init(i2c0, 100 * 1000); //

    // Initialize I2C1 (pins 14/15)
    i2c_init(i2c1, 100 * 1000); // 100kHz

    // Initialize GPIO pins for encoder
    gpio_init(PIN_GPIO_ENCODER_CLOCK);
    gpio_set_dir(PIN_GPIO_ENCODER_CLOCK, GPIO_IN);
    gpio_pull_up(PIN_GPIO_ENCODER_CLOCK);
    
    gpio_init(PIN_GPIO_ENCODER_DATA);
    gpio_set_dir(PIN_GPIO_ENCODER_DATA, GPIO_IN);
    gpio_pull_up(PIN_GPIO_ENCODER_DATA);
    
    gpio_init(PIN_GPIO_ENCODER_BUTTON);
    gpio_set_dir(PIN_GPIO_ENCODER_BUTTON, GPIO_IN);
    gpio_pull_up(PIN_GPIO_ENCODER_BUTTON);

    gpio_set_function(PIN_IC20_SCL, GPIO_FUNC_I2C);
    gpio_set_function(PIN_IC20_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_IC20_SDA);
    gpio_pull_up(PIN_IC20_SCL);
    
    gpio_set_function(PIN_IC12_SCL, GPIO_FUNC_I2C);
    gpio_set_function(PIN_IC12_SDA, GPIO_FUNC_I2C);     
    gpio_pull_up(PIN_IC12_SDA);
    gpio_pull_up(PIN_IC12_SCL);
}