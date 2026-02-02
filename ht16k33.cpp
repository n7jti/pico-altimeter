//(C)  Alan Ludwig 2026, all rights reserved.

#include "ht16k33.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <cstring>

namespace ht16k33 {

// I2C address for HT16K33
constexpr uint8_t HT16K33_I2C_ADDRESS = 0x70;


// System setup register
constexpr uint8_t HT16K33_SYSTEM_SETUP = 0x20;
constexpr uint8_t HT16K33_OSCILLATOR_ON = 0x01;

// Display setup register 
constexpr uint8_t HT16K33_DISPLAY_SETUP = 0x80;
constexpr uint8_t HT16K33_DISPLAY_ON = 0x01;

// Brightness command
constexpr uint8_t HT16K33_BRIGHTNESS_CMD = 0xE0;

// Blink rate values
constexpr uint8_t HT16K33_BLINK_OFF = 0x00;
constexpr uint8_t HT16K33_BLINK_2HZ = 0x02;
constexpr uint8_t HT16K33_BLINK_1HZ = 0x04;
constexpr uint8_t HT16K33_BLINK_HALFHZ = 0x06;

// 7-segment digit patterns (0-9, A-F for hex display)
constexpr uint8_t DIGIT_PATTERNS[16] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F, // 9
    0x77, // A
    0x7C, // b (lowercase to distinguish from 8)
    0x39, // C
    0x5E, // d (lowercase to distinguish from 0)
    0x79, // E
    0x71  // F
};

// Decimal point bit
constexpr uint8_t DECIMAL_POINT = 0x80;

HT16K33::HT16K33(i2c_inst_t* i2c_instance) : i2cAddress(HT16K33_I2C_ADDRESS), i2c(i2c_instance) {
    memset(displayBuffer, 0, sizeof(displayBuffer));
}

void HT16K33::begin() {
    // Turn on the oscillator
    uint8_t data = HT16K33_SYSTEM_SETUP | HT16K33_OSCILLATOR_ON;
    i2c_write_blocking(i2c, i2cAddress, &data, 1, false);
    
    // Turn on the display, no blinking
    data = HT16K33_DISPLAY_SETUP | HT16K33_DISPLAY_ON;
    i2c_write_blocking(i2c, i2cAddress, &data, 1, false);
    
    // Set brightness to maximum
    setBrightness(15);
    
    // Clear the display
    clear();
}

void HT16K33::setBrightness(uint8_t brightness) {
    // Brightness range is 0-15
    if (brightness > 15) {
        brightness = 15;
    }
    
    uint8_t data = HT16K33_BRIGHTNESS_CMD | brightness;
    i2c_write_blocking(i2c, i2cAddress, &data, 1, false);
}

void HT16K33::setBlinkRate(uint8_t rate) {
    uint8_t blinkBits;
    
    switch (rate) {
        case 0:
            blinkBits = HT16K33_BLINK_OFF;
            break;
        case 1:
            blinkBits = HT16K33_BLINK_2HZ;
            break;
        case 2:
            blinkBits = HT16K33_BLINK_1HZ;
            break;
        case 3:
            blinkBits = HT16K33_BLINK_HALFHZ;
            break;
        default:
            blinkBits = HT16K33_BLINK_OFF;
            break;
    }
    
    uint8_t data = HT16K33_DISPLAY_SETUP | HT16K33_DISPLAY_ON | blinkBits;
    i2c_write_blocking(i2c, i2cAddress, &data, 1, false);
}

void HT16K33::displayDigit(uint8_t position, uint8_t digit, bool dot) {
    // Validate position (0-3 for 4-digit display)
    if (position >= 4) {
        return;
    }
    
    // Validate digit (0-15 for hex)
    if (digit > 15) {
        return;
    }
    
    // Get the pattern for the digit
    uint8_t pattern = DIGIT_PATTERNS[digit];
    
    // Add decimal point if requested
    if (dot) {
        pattern |= DECIMAL_POINT;
    }
    
    // Write to the display buffer
    // Adafruit LED backpack has colon at address 0x04 (between positions 1 and 2)
    // Position 0: address 0x00
    // Position 1: address 0x02
    // Colon:      address 0x04
    // Position 2: address 0x06
    // Position 3: address 0x08
    uint8_t address;
    if (position < 2) {
        address = position * 2;
    } else {
        address = (position * 2) + 2;  // Skip the colon at 0x04
    }
    displayBuffer[address] = pattern;
}

void HT16K33::writeDisplay() {
    // The HT16K33 expects the display RAM address (0x00) followed by 16 bytes
    uint8_t buffer[17];
    buffer[0] = 0x00; // Start at address 0x00
    memcpy(buffer + 1, displayBuffer, sizeof(displayBuffer));
    
    i2c_write_blocking(i2c, i2cAddress, buffer, sizeof(buffer), false);
}

void HT16K33::clear() {
    memset(displayBuffer, 0, sizeof(displayBuffer));
    writeDisplay();
}



}  // namespace ht16k33