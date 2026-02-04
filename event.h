// (C) Alan Ludwig 2026, all rights reserved.
#pragma once

#include <cstdint>

namespace event {

// Event types
enum class EventType : uint8_t {
    None = 0,
    Timer,              // Periodic timer tick
    EncoderChange,      // Encoder position changed
    ButtonPress,        // Encoder button pressed
};

// Event structure
struct Event {
    EventType type;
    int32_t data;       // Optional data (e.g., encoder delta, timer id)
    
    Event() : type(EventType::None), data(0) {}
    Event(EventType t, int32_t d = 0) : type(t), data(d) {}
};

// Initialize the event queue
void initEventQueue();

// Queue an event (IRQ-safe)
// Returns true if event was queued, false if queue is full
bool queueEvent(const Event& event);

// Queue an event from ISR context (IRQ-safe, non-blocking)
bool queueEventFromISR(const Event& event);

// Wait for and retrieve an event (blocking)
Event waitForEvent();

// Check if there's an event available (non-blocking)
bool hasEvent();

// Get an event if available (non-blocking)
// Returns event with type None if queue is empty
Event tryGetEvent();

}  // namespace event
