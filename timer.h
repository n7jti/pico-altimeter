// (C) Alan Ludwig 2026, all rights reserved.
#pragma once

#include <cstdint>

namespace timer {

// Initialize the hardware timer to fire at the specified interval
// intervalMs: timer period in milliseconds
void initTimer(uint32_t intervalMs = 250);

// Stop the timer
void stopTimer();

// Start the timer (if stopped)
void startTimer();

// Change the timer interval
void setInterval(uint32_t intervalMs);

}  // namespace timer
