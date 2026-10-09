#include "debounce.h"

void debounce_init(debounce_state_t *state, uint64_t debounce_ns, int initial_value)
{
    if (state == 0) return;
    state->initialized = true;
    state->stable_value = initial_value ? 1 : 0;
    state->candidate_value = initial_value ? 1 : 0;
    state->candidate_since_ns = 0;
    state->debounce_ns = debounce_ns;
}

bool debounce_update(debounce_state_t *state, int sample_value, uint64_t timestamp_ns,
                     int *accepted_value)
{
    int value;
    if (state == 0 || accepted_value == 0 || (sample_value != 0 && sample_value != 1)) return false;
    value = sample_value;
    if (!state->initialized) {
        state->initialized = true;
        state->stable_value = value;
        state->candidate_value = value;
        state->candidate_since_ns = timestamp_ns;
        return false;
    }
    if (value == state->stable_value) {
        state->candidate_value = value;
        state->candidate_since_ns = timestamp_ns;
        return false;
    }
    if (value != state->candidate_value) {
        state->candidate_value = value;
        state->candidate_since_ns = timestamp_ns;
        return false;
    }
    if (timestamp_ns < state->candidate_since_ns ||
        timestamp_ns - state->candidate_since_ns < state->debounce_ns) return false;
    state->stable_value = value;
    *accepted_value = value;
    return true;
}
