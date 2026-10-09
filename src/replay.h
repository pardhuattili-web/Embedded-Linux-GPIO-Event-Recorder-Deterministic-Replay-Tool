#ifndef REPLAY_H
#define REPLAY_H

#include <stdbool.h>
#include "event.h"

typedef int (*replay_callback_t)(const gpio_event_t *event, void *context);

typedef struct {
    double speed;
    bool no_sleep;
    bool verbose;
} replay_options_t;

/* Returns 0 on success; a malformed log or callback failure returns -1. */
int replay_csv(FILE *input, const replay_options_t *options,
               replay_callback_t callback, void *context);

#endif
