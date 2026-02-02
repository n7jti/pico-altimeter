// (C) Alan Ludwig 2026, all rights reserved.

#include "bmp390.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <cmath>
#include <cstring>
#include <cstdio>

extern "C" {
#include "bmp3.h"
}

namespace bmp390 {

// Structure to hold I2C instance and address for callbacks
struct I2CContext {
    i2c_inst_t* i2c;
    uint8_t address;
};

// I2C read callback for BMP3 API
static BMP3_INTF_RET_TYPE i2c_read(uint8_t reg_addr, uint8_t *read_data, uint32_t len, void *intf_ptr) {
    I2CContext* ctx = static_cast<I2CContext*>(intf_ptr);
    
    // Write register address
    int result = i2c_write_blocking(ctx->i2c, ctx->address, &reg_addr, 1, true);
    if (result < 0) {
        return BMP3_E_COMM_FAIL;
    }
    
    // Read data
    result = i2c_read_blocking(ctx->i2c, ctx->address, read_data, len, false);
    if (result < 0) {
        return BMP3_E_COMM_FAIL;
    }
    
    return BMP3_OK;
}

// I2C write callback for BMP3 API
static BMP3_INTF_RET_TYPE i2c_write(uint8_t reg_addr, const uint8_t *write_data, uint32_t len, void *intf_ptr) {
    I2CContext* ctx = static_cast<I2CContext*>(intf_ptr);
    
    // Create buffer with register address followed by data
    uint8_t buffer[len + 1];
    buffer[0] = reg_addr;
    memcpy(buffer + 1, write_data, len);
    
    int result = i2c_write_blocking(ctx->i2c, ctx->address, buffer, len + 1, false);
    if (result < 0) {
        return BMP3_E_COMM_FAIL;
    }
    
    return BMP3_OK;
}

// Delay callback for BMP3 API
static void delay_us(uint32_t period, void *intf_ptr) {
    (void)intf_ptr;
    sleep_us(period);
}

BMP390::BMP390(i2c_inst_t* i2c, uint8_t address) 
    : i2c(i2c), i2cAddress(address), temperature(0.0), pressure(0.0), 
      seaLevelPressurePa(101325.0), dev(nullptr) {
}

bool BMP390::begin() {
    printf("BMP390::begin() - Initializing at address 0x%02X\n", i2cAddress);
    
    // Allocate BMP3 device structure and I2C context
    bmp3_dev* bmp3 = new bmp3_dev();
    I2CContext* ctx = new I2CContext{i2c, i2cAddress};
    
    memset(bmp3, 0, sizeof(bmp3_dev));
    
    // Configure the device structure
    bmp3->intf = BMP3_I2C_INTF;
    bmp3->intf_ptr = ctx;
    bmp3->read = i2c_read;
    bmp3->write = i2c_write;
    bmp3->delay_us = delay_us;
    
    // Initialize the sensor
    int8_t rslt = bmp3_init(bmp3);
    if (rslt != BMP3_OK) {
        printf("BMP390: bmp3_init failed with error %d\n", rslt);
        printf("  Chip ID read: 0x%02X (expected 0x50 for BMP388 or 0x60 for BMP390)\n", bmp3->chip_id);
        delete ctx;
        delete bmp3;
        return false;
    }
    
    printf("BMP390: Chip ID = 0x%02X\n", bmp3->chip_id);
    
    // Configure sensor settings
    // Note: ODR must be compatible with oversampling settings
    // With 4x press OS and 2x temp OS, using 12.5Hz for ~100ms updates
    bmp3_settings settings = {};
    settings.press_en = BMP3_ENABLE;
    settings.temp_en = BMP3_ENABLE;
    settings.odr_filter.press_os = BMP3_OVERSAMPLING_4X;
    settings.odr_filter.temp_os = BMP3_OVERSAMPLING_2X;
    settings.odr_filter.odr = BMP3_ODR_12_5_HZ;
    settings.odr_filter.iir_filter = BMP3_IIR_FILTER_COEFF_3;
    
    uint32_t settings_sel = BMP3_SEL_PRESS_EN | BMP3_SEL_TEMP_EN | 
                            BMP3_SEL_PRESS_OS | BMP3_SEL_TEMP_OS | 
                            BMP3_SEL_ODR | BMP3_SEL_IIR_FILTER;
    
    rslt = bmp3_set_sensor_settings(settings_sel, &settings, bmp3);
    if (rslt != BMP3_OK) {
        printf("BMP390: bmp3_set_sensor_settings failed with error %d\n", rslt);
        delete ctx;
        delete bmp3;
        return false;
    }
    
    // Set to normal mode
    settings.op_mode = BMP3_MODE_NORMAL;
    rslt = bmp3_set_op_mode(&settings, bmp3);
    if (rslt != BMP3_OK) {
        printf("BMP390: bmp3_set_op_mode failed with error %d\n", rslt);
        delete ctx;
        delete bmp3;
        return false;
    }
    
    printf("BMP390: Initialized successfully!\n");
    dev = bmp3;
    return true;
}

bool BMP390::readSensor() {
    if (!dev) {
        return false;
    }
    
    bmp3_dev* bmp3 = static_cast<bmp3_dev*>(dev);
    bmp3_data data = {};
    
    int8_t rslt = bmp3_get_sensor_data(BMP3_PRESS_TEMP, &data, bmp3);
    if (rslt != BMP3_OK) {
        return false;
    }
    
    temperature = data.temperature;
    pressure = data.pressure;
    
    return true;
}

double BMP390::getAltitudeMeters(double seaLevelPressure) const {
    // International barometric formula
    // altitude = 44330 * (1 - (P/P0)^(1/5.255))
    if (pressure <= 0 || seaLevelPressure <= 0) {
        return 0.0;
    }
    
    return 44330.0 * (1.0 - pow(pressure / seaLevelPressure, 0.1903));
}

double BMP390::getAltitudeFeet(double seaLevelPressure) const {
    // Convert meters to feet
    return getAltitudeMeters(seaLevelPressure) * 3.28084;
}

}  // namespace bmp390
