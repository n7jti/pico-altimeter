#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pins.h"
#include "ht16k33.h"
#include "bmp390.h"
#include "event.h"
#include "timer.h"
#include "encoder.h"

// State machine states
enum class DeviceState {
    Altimeter,  // Display altitude
    Setting     // Display/adjust sea level pressure
};

// Conversion constant: 1 inHg = 3386.389 Pa
constexpr double INHG_TO_PA = 3386.389;

// Default sea level pressure: 29.92 inHg = 2992 in encoder units
constexpr int32_t DEFAULT_PRESSURE_INHG_X100 = 2992;

// Global display and sensor pointers for event handlers
static ht16k33::HT16K33* g_display = nullptr;
static bmp390::BMP390* g_sensor = nullptr;
static DeviceState g_state = DeviceState::Altimeter;

// Update the setting display (used by both timer and encoder events)
void updateDisplay() {
    if (!g_sensor || !g_display) return;

    switch(g_state) {
        case DeviceState::Altimeter: {    
            if (!g_sensor->readSensor()) {
                printf("Sensor read failed!\n");
                return;
            }
            int altitudeFeet = g_sensor->getAltitudeFeet();
            g_display->displayNumber(altitudeFeet);
            g_display->setColon(false);
            g_display->writeDisplay();
            break;
        }
        case DeviceState::Setting: {
            int32_t position = encoder::getPosition();
            // Display as inHg with two decimal places
            g_display->displayNumber(position, 2);
            g_display->setColon(true);
            g_display->writeDisplay();
            break;
        }
        default:
            printf("Unknown state in updateDisplay\n");
            break;
    }
}

// Handle timer event based on current state
void handleTimerEvent() {
    if (!g_sensor || !g_display) return;
    updateDisplay(); 
}

// Handle encoder rotation event
void handleEncoderEvent(int32_t delta) {
    int32_t position = encoder::getPosition();
    printf("Encoder: delta=%d, position=%d, state=%d\n", (int)delta, (int)position, (int)g_state);
}

// Handle button press event - toggle state
void handleButtonEvent() {
    if (!g_sensor || !g_display) return;

    switch(g_state) {
        case DeviceState::Altimeter:
            printf("Button pressed: switching to SETTING mode\n");
            g_state = DeviceState::Setting;
            break;
        case DeviceState::Setting:
            printf("Button pressed: switching to ALTIMETER mode\n");
            {
                // Update sensor with new sea level pressure
                double seaLevelPa = encoder::getPascals();
                g_sensor->setSeaLevelPressure(seaLevelPa);
                printf("Updated sea level pressure to %.2f Pa (%.2f inHg)\n", seaLevelPa, encoder::getPosition() / 100.0);
            }
            g_state = DeviceState::Altimeter;
            break;
        default:
            printf("Unknown state in button handler\n");
            break;
    }
}

int main()
{
    stdio_init_all();
    initializePins();
    
    // Initialize event queue first
    event::initEventQueue();
    
    // Initialize display with i2c1 instance
    ht16k33::HT16K33 display(i2c1);
    display.begin();
    g_display = &display;

    // Test the display
    display.testDisplay();

    // Try to initialize BMP390 sensor
    printf("Trying BMP390 at address 0x77 on i2c0...\n");
    bmp390::BMP390 sensor(i2c0, 0x77);
    if (!sensor.begin()) {
        printf("Failed to initialize BMP390 sensor!\n");
        display.displayDigit(0, 0x0E); // Display 'E' for error
        display.displayDigit(1, 0x0E);
        display.displayDigit(2, 0x0E);
        display.displayDigit(3, 0x0E);
        display.writeDisplay();
        return -1;
    }
    g_sensor = &sensor;
    printf("BMP390 sensor initialized successfully!\n");

    // Initialize encoder with default pressure setting (29.92 inHg)
    encoder::initEncoder();
    encoder::setPosition(DEFAULT_PRESSURE_INHG_X100);
    printf("Encoder initialized to %d (%.2f inHg)\n", 
           DEFAULT_PRESSURE_INHG_X100, DEFAULT_PRESSURE_INHG_X100 / 100.0);

    // Start the timer (250ms = 4 updates per second)
    timer::initTimer(250);
    printf("Timer started\n");

    printf("Entering event loop in ALTIMETER mode...\n");
    printf("Press button to switch to SETTING mode\n");

    // Main event loop
    while (true) {
        event::Event evt = event::waitForEvent();
        
        switch (evt.type) {
            case event::EventType::Timer:
                handleTimerEvent();
                break;
                
            case event::EventType::EncoderChange:
                handleEncoderEvent(evt.data);
                break;
                
            case event::EventType::ButtonPress:
                handleButtonEvent();
                break;
                
            case event::EventType::None:
            default:
                // Should not happen
                printf("Received unknown event type %d in main loop\n", (int)evt.type);
                break;
        }
    }
}
