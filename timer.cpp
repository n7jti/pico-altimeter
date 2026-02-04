// (C) Alan Ludwig 2026, all rights reserved.

#include "timer.h"
#include "event.h"
#include <pico/stdlib.h>
#include <hardware/timer.h>

namespace timer {

// Timer state
static repeating_timer_t repeatingTimer;
static bool timerRunning = false;

// Timer callback - called from IRQ context
static bool timerCallback(repeating_timer_t *rt) {
    // Queue a timer event
    event::queueEventFromISR(event::Event(event::EventType::Timer));
    return true;  // Keep repeating
}

void initTimer(uint32_t intervalMs) {
    // Cancel any existing timer
    if (timerRunning) {
        cancel_repeating_timer(&repeatingTimer);
    }
    
    // Start repeating timer (negative interval = delay between callbacks)
    // Positive interval = interval from start of callback to start of next
    timerRunning = add_repeating_timer_ms(-(int32_t)intervalMs, timerCallback, nullptr, &repeatingTimer);
}

void stopTimer() {
    if (timerRunning) {
        cancel_repeating_timer(&repeatingTimer);
        timerRunning = false;
    }
}

void startTimer() {
    if (!timerRunning) {
        // Restart with default 250ms interval
        initTimer(250);
    }
}

void setInterval(uint32_t intervalMs) {
    // Stop and restart with new interval
    stopTimer();
    initTimer(intervalMs);
}

}  // namespace timer
