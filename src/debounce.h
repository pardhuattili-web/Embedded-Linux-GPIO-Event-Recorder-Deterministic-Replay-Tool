#ifndef DEBOUNCE_H
#define DEBOUNCE_H

#include <stdbool.h>
#include <stdint.h>
#include "event.h"

typedef struct {
    bool initialized;
    int stable_value;
    int candidate_value;
    uint64_t candidate_since_ns;
    uint64_t debounce_ns;
} debounce_state_t;

void debounce_init(debounce_state_t *state, uint64_t debounce_ns, int initial_value);
bool debounce_update(debounce_state_t *state, int sample_value, uint64_t timestamp_ns,
                     int *accepted_value);

#endif
