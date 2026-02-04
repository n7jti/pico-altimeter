// (C) Alan Ludwig 2026, all rights reserved.

#include <pico/stdlib.h>
#include <pico/critical_section.h>
#include <hardware/gpio.h>
#include "encoder.h"
#include "event.h"
#include "pins.h"

namespace encoder {

// Critical section for thread-safe access to encoder state
static critical_section_t encoderCriticalSection;

// Encoder state (protected by critical section)
static volatile int32_t encoderPosition = 0;
static volatile bool buttonPressedFlag = false;

// Debounce timing for button
static volatile uint32_t lastButtonTime = 0;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 50;

// GPIO interrupt callback
static void gpio_callback(uint gpio, uint32_t events) {
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    
    if (gpio == PIN_GPIO_ENCODER_CLOCK) {
        // State machine for quadrature decoding
        // Only trigger on clock falling edge for single-step detection
        if (events & GPIO_IRQ_EDGE_FALL) {  
            if ((currentTime - lastButtonTime) > BUTTON_DEBOUNCE_MS) {
                // Read data pin to determine direction
                uint8_t data = gpio_get(PIN_GPIO_ENCODER_DATA);
                
                int32_t delta;
                if (data) {
                    encoderPosition++;  // Clockwise
                    delta = 1;
                } else {
                    encoderPosition--;  // Counter-clockwise
                    delta = -1;
                }

                lastButtonTime = currentTime;

                // Queue encoder change event with delta
                event::queueEventFromISR(event::Event(event::EventType::EncoderChange, delta));
            }
            
            
        }
    }
    else if (gpio == PIN_GPIO_ENCODER_BUTTON) {
        // Button interrupt with debouncing
        if (events & GPIO_IRQ_EDGE_FALL) {  // Button pressed (active low)
            if ((currentTime - lastButtonTime) > BUTTON_DEBOUNCE_MS) {
                buttonPressedFlag = true;
                lastButtonTime = currentTime;
                
                // Queue button press event
                event::queueEventFromISR(event::Event(event::EventType::ButtonPress));
            }
        }
    }
}

void initEncoder() {
    // Initialize the critical section
    critical_section_init(&encoderCriticalSection);
    
    // Set up interrupts for encoder clock pin (falling edge for single-step)
    gpio_set_irq_enabled_with_callback(PIN_GPIO_ENCODER_CLOCK, 
                                        GPIO_IRQ_EDGE_FALL, 
                                        true, 
                                        &gpio_callback);
    
    // Set up interrupt for button (falling edge = press)
    gpio_set_irq_enabled(PIN_GPIO_ENCODER_BUTTON, 
                         GPIO_IRQ_EDGE_FALL, 
                         true);
}

int32_t getPosition() {
    critical_section_enter_blocking(&encoderCriticalSection);
    int32_t pos = encoderPosition;
    critical_section_exit(&encoderCriticalSection);
    return pos;
}

void setPosition(int32_t position) {
    critical_section_enter_blocking(&encoderCriticalSection);
    encoderPosition = position;
    critical_section_exit(&encoderCriticalSection);
}

bool wasButtonPressed() {
    critical_section_enter_blocking(&encoderCriticalSection);
    bool pressed = buttonPressedFlag;
    buttonPressedFlag = false;  // Clear flag after reading
    critical_section_exit(&encoderCriticalSection);
    return pressed;
}

bool isButtonPressed() {
    // Active low - returns true when button is pressed
    return !gpio_get(PIN_GPIO_ENCODER_BUTTON);
}

// Convert encoder count (inHg * 100) to Pascals
double getPascals() {
    int32_t encoderCount = getPosition();
    constexpr double INHG_TO_PA = 3386.39;  // 1 inHg = 3386.39 Pa
    double inHg = encoderCount / 100.0;
    return inHg * INHG_TO_PA;
}

}  // namespace encoder

