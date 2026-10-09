#define _POSIX_C_SOURCE 200809L
#include "event.h"

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

int event_write_csv_header(FILE *stream)
{
    if (stream == NULL) return -1;
    return fprintf(stream, "sequence,elapsed_ns,line_offset,edge,value\n") < 0 ? -1 : 0;
}

const char *event_edge_name(event_edge_t edge)
{
    switch (edge) {
    case EVENT_EDGE_RISING: return "rising";
    case EVENT_EDGE_FALLING: return "falling";
    default: return "invalid";
    }
}

int event_edge_parse(const char *text, event_edge_t *edge)
{
    if (text == NULL || edge == NULL) return -1;
    if (strcmp(text, "rising") == 0) { *edge = EVENT_EDGE_RISING; return 0; }
    if (strcmp(text, "falling") == 0) { *edge = EVENT_EDGE_FALLING; return 0; }
    return -1;
}

int event_write_csv(FILE *stream, const gpio_event_t *event)
{
    if (stream == NULL || event == NULL || event->value < 0 || event->value > 1 ||
        (event->edge != EVENT_EDGE_RISING && event->edge != EVENT_EDGE_FALLING)) return -1;
    return fprintf(stream, "%" PRIu64 ",%" PRIu64 ",%u,%s,%d\n",
                   event->sequence, event->elapsed_ns, event->line_offset,
                   event_edge_name(event->edge), event->value) < 0 ? -1 : 0;
}

static int parse_u64(const char *s, uint64_t *out)
{
    char *end = NULL;
    unsigned long long v;
    if (s == NULL || *s == '\0' || *s == '-') return -1;
    errno = 0;
    v = strtoull(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return -1;
    *out = (uint64_t)v;
    return 0;
}

int event_read_csv(FILE *stream, gpio_event_t *event)
{
    char line[256];
    char edge_text[16];
    char value_text[8];
    char *fields[5];
    char *cursor;
    char *comma;
    uint64_t seq, elapsed, line_num;
    event_edge_t edge;
    if (stream == NULL || event == NULL) return -1;
    if (fgets(line, sizeof(line), stream) == NULL) return feof(stream) ? 0 : -1;
    if (strchr(line, '\n') == NULL && !feof(stream)) {
        int ch;
        while ((ch = fgetc(stream)) != '\n' && ch != EOF) {}
        return -1;
    }
    line[strcspn(line, "\r\n")] = '\0';
    cursor = line;
    for (int i = 0; i < 4; ++i) {
        fields[i] = cursor;
        comma = strchr(cursor, ',');
        if (comma == NULL) return -1;
        *comma = '\0';
        cursor = comma + 1;
    }
    fields[4] = cursor;
    if (strchr(fields[4], ',') != NULL) return -1;
    if (parse_u64(fields[0], &seq) != 0 ||
        parse_u64(fields[1], &elapsed) != 0 ||
        parse_u64(fields[2], &line_num) != 0 ||
        line_num > UINT32_MAX ||
        event_edge_parse(fields[3], &edge) != 0) return -1;
    (void)strncpy(edge_text, fields[3], sizeof(edge_text)-1);
    edge_text[sizeof(edge_text)-1] = '\0';
    (void)strncpy(value_text, fields[4], sizeof(value_text)-1);
    value_text[sizeof(value_text)-1] = '\0';
    if ((strcmp(value_text, "0") != 0 && strcmp(value_text, "1") != 0) ||
        ((edge == EVENT_EDGE_RISING) != (strcmp(value_text, "1") == 0))) return -1;
    event->sequence = seq;
    event->elapsed_ns = elapsed;
    event->line_offset = (unsigned int)line_num;
    event->edge = edge;
    event->value = value_text[0] - '0';
    return 1;
}
