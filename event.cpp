// (C) Alan Ludwig 2026, all rights reserved.

#include "event.h"
#include <pico/stdlib.h>
#include <pico/util/queue.h>

namespace event {

// Queue configuration
constexpr size_t QUEUE_SIZE = 32;

// Pico SDK queue for events
static queue_t eventQueue;

void initEventQueue() {
    queue_init(&eventQueue, sizeof(Event), QUEUE_SIZE);
}

bool queueEvent(const Event& event) {
    return queue_try_add(&eventQueue, &event);
}

bool queueEventFromISR(const Event& event) {
    // Pico SDK queue is ISR-safe
    return queue_try_add(&eventQueue, &event);
}

Event waitForEvent() {
    Event event;
    queue_remove_blocking(&eventQueue, &event);
    return event;
}

bool hasEvent() {
    return !queue_is_empty(&eventQueue);
}

Event tryGetEvent() {
    Event event;
    if (!queue_try_remove(&eventQueue, &event)) {
        event.type = EventType::None;
        event.data = 0;
    }
    return event;
}

}  // namespace event
