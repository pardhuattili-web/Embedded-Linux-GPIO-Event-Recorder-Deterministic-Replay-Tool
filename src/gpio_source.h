#ifndef GPIO_SOURCE_H
#define GPIO_SOURCE_H

#include <stdbool.h>
#include <stdint.h>
#include "event.h"

typedef struct {
    unsigned int line_offset;
    uint64_t duration_ms;
    uint64_t debounce_ms;
    bool verbose;
} demo_source_options_t;

/* Portable sample generator. Returns accepted-event count or -1 on I/O failure. */
int gpio_source_demo(FILE *output, const demo_source_options_t *options);

#endif
