// (C) Alan Ludwig 2026, all rights reserved.

#pragma once

#include <cstdint>

typedef struct i2c_inst i2c_inst_t;

namespace bmp390 {

class BMP390 {
public:
    BMP390(i2c_inst_t* i2c, uint8_t address = 0x77);
    
    // Initialize the sensor, returns true on success
    bool begin();
    
    // Read sensor data
    bool readSensor();
    
    // Get the last read temperature in degrees Celsius
    double getTemperature() const { return temperature; }
    
    // Get the last read pressure in Pascals
    double getPressure() const { return pressure; }
    
    // Calculate altitude from pressure using the barometric formula
    // seaLevelPressure should be in Pascals (default 101325 Pa = 1013.25 hPa)
    double getAltitudeMeters(double seaLevelPressure) const;
    
    // Get altitude in feet
    double getAltitudeFeet(double seaLevelPressure) const;
    
    // Set the reference sea level pressure for altitude calculations
    void setSeaLevelPressure(double pressure) { seaLevelPressurePa = pressure; }
    
    // Get altitude using stored sea level pressure (101325 Pa default)
    double getAltitudeMeters() const { return getAltitudeMeters(seaLevelPressurePa); }
    double getAltitudeFeet() const { return getAltitudeFeet(seaLevelPressurePa); }

private:
    i2c_inst_t* i2c;
    uint8_t i2cAddress;
    double temperature;  // Celsius
    double pressure;     // Pascals
    double seaLevelPressurePa;
    
    // Opaque pointer to BMP3 device structure
    void* dev;
};

}  // namespace bmp390
