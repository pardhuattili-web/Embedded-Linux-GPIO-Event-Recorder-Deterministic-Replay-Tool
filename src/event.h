#ifndef EVENT_H
#define EVENT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
    EVENT_EDGE_RISING = 1,
    EVENT_EDGE_FALLING = 0
} event_edge_t;

typedef struct {
    uint64_t sequence;
    uint64_t elapsed_ns;
    unsigned int line_offset;
    event_edge_t edge;
    int value;
} gpio_event_t;

int event_write_csv_header(FILE *stream);
int event_write_csv(FILE *stream, const gpio_event_t *event);
int event_read_csv(FILE *stream, gpio_event_t *event);
const char *event_edge_name(event_edge_t edge);
int event_edge_parse(const char *text, event_edge_t *edge);

#endif
