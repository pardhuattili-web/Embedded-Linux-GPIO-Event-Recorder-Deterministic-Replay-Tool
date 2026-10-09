#define _POSIX_C_SOURCE 200809L
#include "replay.h"

#include <errno.h>
#include <inttypes.h>
#include <time.h>

static int sleep_ns(uint64_t ns)
{
    struct timespec req = {(time_t)(ns / 1000000000ULL), (long)(ns % 1000000000ULL)};
    while (nanosleep(&req, &req) != 0) {
        if (errno != EINTR) return -1;
    }
    return 0;
}

int replay_csv(FILE *input, const replay_options_t *options,
               replay_callback_t callback, void *context)
{
    gpio_event_t event, previous = {0};
    int rc;
    bool have_previous = false;
    if (input == NULL || options == NULL || callback == NULL ||
        options->speed <= 0.0) return -1;
    while ((rc = event_read_csv(input, &event)) > 0) {
        if (have_previous) {
            uint64_t delta;
            if (event.sequence <= previous.sequence || event.elapsed_ns < previous.elapsed_ns) return -1;
            delta = event.elapsed_ns - previous.elapsed_ns;
            if (!options->no_sleep) {
                long double scaled = (long double)delta / options->speed;
                if (scaled > (long double)UINT64_MAX) return -1;
                if (sleep_ns((uint64_t)scaled) != 0) return -1;
            }
        }
        if (callback(&event, context) != 0) return -1;
        previous = event;
        have_previous = true;
    }
    return rc < 0 ? -1 : 0;
}
