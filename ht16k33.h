// (C) 2026 Alan Ludwig, all rights reserved.

#pragma once

#include <cstdint>

typedef struct i2c_inst i2c_inst_t;

namespace ht16k33 {

class HT16K33 {
public:
    HT16K33(i2c_inst_t* i2c);
    void begin();
    void setBrightness(uint8_t brightness);
    void setBlinkRate(uint8_t rate);
    void displayDigit(uint8_t position, uint8_t digit, bool dot = false);
    void writeDisplay();
    void clear();

private:
    uint8_t displayBuffer[16];
    uint8_t i2cAddress;
    i2c_inst_t* i2c;
};

}  // namespace ht16k33
