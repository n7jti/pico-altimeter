// (C) Alan Ludwig 2026, all rights reserved.
#pragma once

#include <cstdint>

namespace encoder {

// Initialize the encoder interrupt handlers
// Call this AFTER initializePins() has set up the GPIO
void initEncoder();

// Get the current encoder position (thread/IRQ safe)
int32_t getPosition();

// Convert encoder count (inHg * 100) to Pascals
double getPascals();

// Set the encoder position (thread/IRQ safe)
void setPosition(int32_t position);

// Check if button was pressed (clears the flag when read)
bool wasButtonPressed();

// Check current button state (true = pressed, active low)
bool isButtonPressed();



}  // namespace encoder

