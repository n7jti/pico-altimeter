#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pins.h"
#include "ht16k33.h"
#include "bmp390.h"

// Scan I2C bus for devices
void i2c_scan(i2c_inst_t* i2c, const char* bus_name) {
    printf("\nScanning %s...\n", bus_name);
    bool found = false;
    for (int addr = 0x08; addr < 0x78; addr++) {
        uint8_t data;
        int ret = i2c_read_blocking(i2c, addr, &data, 1, false);
        if (ret >= 0) {
            printf("  Found device at address 0x%02X\n", addr);
            found = true;
        }
    }
    if (!found) {
        printf("  No devices found!\n");
    }
}

int main()
{
    stdio_init_all();
    initializePins();
    
    // Initialize display with i2c1 instance
    ht16k33::HT16K33 display(i2c1);
    display.begin();

    // Test the display
    display.testDisplay();

    // Scan both I2C buses to find devices
    printf("\n--- I2C Bus Scan ---\n");
    i2c_scan(i2c0, "I2C0 (pins 12/13)");
    i2c_scan(i2c1, "I2C1 (pins 14/15)");
    printf("--- End Scan ---\n\n");

    // Try to initialize BMP390 sensor
    printf("Trying BMP390 at address 0x77 on i2c0...\n");
    bmp390::BMP390 sensor(i2c0, 0x77);
    if (!sensor.begin()) {
        printf("Failed to initialize BMP390 sensor!\n");
        display.displayDigit(0, 0x0E); // Display 'E' for error
        display.displayDigit(1, 0x0E); // Display 'E' for error
        display.displayDigit(2, 0x0E); // Display 'E' for error
        display.displayDigit(3, 0x0E); // Display 'E' for error

        return -1;
    }
    printf("BMP390 sensor initialized successfully!\n");

    while (true) {
        if (sensor.readSensor()) {
            double temperature = sensor.getTemperature();
            double pressure = sensor.getPressure();
            double altitudeFeet = sensor.getAltitudeFeet();
            
            printf("Temp: %.2f C, Pressure: %.2f Pa, Altitude: %.1f ft\n", 
                   temperature, pressure, altitudeFeet);
            
            // Display altitude in feet on the LED (rounded to integer)
            int altDisplay = static_cast<int>(altitudeFeet + 0.5);
            display.displayNumber(altDisplay);
        } else {
            printf("Failed to read sensor data\n");
        }
        
        sleep_ms(250);  // Update 4 times per second
    }
}
