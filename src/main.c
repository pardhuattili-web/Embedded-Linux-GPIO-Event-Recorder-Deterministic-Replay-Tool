#define _POSIX_C_SOURCE 200809L
#include "debounce.h"
#include "event.h"
#include "gpio_source.h"
#include "replay.h"

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(FILE *out)
{
    fprintf(out,
        "Embedded Linux GPIO Event Recorder & Deterministic Replay Tool\n"
        "Usage:\n"
        "  gpio-recorder record --source demo --output FILE [options]\n"
        "  gpio-recorder replay --input FILE [--speed N] [--no-sleep]\n"
        "Options for record: --duration-ms N --line N --debounce-ms N --verbose\n"
        "Replay options:     --speed N --no-sleep\n"
        "  --help             Show this help\n\n"
        "The demo source requires no GPIO hardware. Live GPIO capture is a board-specific adapter.\n");
}

static int parse_u64_arg(const char *s, uint64_t *out)
{
    char *end = NULL;
    unsigned long long n;
    if (s == NULL || *s == '\0' || *s == '-') return -1;
    errno = 0;
    n = strtoull(s, &end, 10);
    if (errno || end == s || *end != '\0') return -1;
    *out = (uint64_t)n;
    return 0;
}

static int replay_print(const gpio_event_t *e, void *unused)
{
    (void)unused;
    printf("%" PRIu64 " +%" PRIu64 " ns line=%u edge=%s value=%d\n",
           e->sequence, e->elapsed_ns, e->line_offset, event_edge_name(e->edge), e->value);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        usage(stdout);
        return argc < 2 ? 2 : 0;
    }
    if (strcmp(argv[1], "record") == 0) {
        const char *source = "demo";
        const char *output_path = NULL;
        uint64_t duration = 1500, line = 17, debounce = 25;
        bool verbose = false;
        FILE *out;
        int count;
        for (int i = 2; i < argc; ++i) {
            if (strcmp(argv[i], "--source") == 0 && i + 1 < argc) source = argv[++i];
            else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) output_path = argv[++i];
            else if (strcmp(argv[i], "--duration-ms") == 0 && i + 1 < argc) {
                if (parse_u64_arg(argv[++i], &duration) != 0) { fprintf(stderr, "invalid duration\n"); return 2; }
            } else if (strcmp(argv[i], "--line") == 0 && i + 1 < argc) {
                if (parse_u64_arg(argv[++i], &line) != 0 || line > UINT32_MAX) { fprintf(stderr, "invalid line offset\n"); return 2; }
            } else if (strcmp(argv[i], "--debounce-ms") == 0 && i + 1 < argc) {
                if (parse_u64_arg(argv[++i], &debounce) != 0 || debounce > 60000) { fprintf(stderr, "invalid debounce interval\n"); return 2; }
            } else if (strcmp(argv[i], "--verbose") == 0) verbose = true;
            else { fprintf(stderr, "unknown/incomplete option: %s\n", argv[i]); usage(stderr); return 2; }
        }
        if (output_path == NULL) { fprintf(stderr, "--output is required\n"); return 2; }
        if (strcmp(source, "demo") != 0) {
            fprintf(stderr, "unsupported source '%s'; this build supports --source demo.\n", source);
            return 2;
        }
        out = fopen(output_path, "w");
        if (out == NULL) { perror(output_path); return 1; }
        {
            demo_source_options_t options = {
                .line_offset = (unsigned int)line, .duration_ms = duration,
                .debounce_ms = debounce, .verbose = verbose
            };
            count = gpio_source_demo(out, &options);
        }
        if (fclose(out) != 0) { perror("close output"); return 1; }
        if (count < 0) { fprintf(stderr, "recording failed\n"); return 1; }
        printf("Wrote %d accepted demo events to %s\n", count, output_path);
        return 0;
    }
    if (strcmp(argv[1], "replay") == 0) {
        const char *input_path = NULL;
        replay_options_t options = {.speed = 1.0, .no_sleep = false, .verbose = false};
        FILE *in;
        for (int i = 2; i < argc; ++i) {
            if (strcmp(argv[i], "--input") == 0 && i + 1 < argc) input_path = argv[++i];
            else if (strcmp(argv[i], "--speed") == 0 && i + 1 < argc) {
                char *end = NULL;
                errno = 0;
                options.speed = strtod(argv[++i], &end);
                if (errno || end == argv[i] || *end != '\0' || options.speed <= 0.0) {
                    fprintf(stderr, "speed must be a positive number\n"); return 2;
                }
            } else if (strcmp(argv[i], "--no-sleep") == 0) options.no_sleep = true;
            else { fprintf(stderr, "unknown/incomplete option: %s\n", argv[i]); usage(stderr); return 2; }
        }
        if (input_path == NULL) { fprintf(stderr, "--input is required\n"); return 2; }
        in = fopen(input_path, "r");
        if (in == NULL) { perror(input_path); return 1; }
        {
            char first[256];
            if (fgets(first, sizeof(first), in) == NULL ||
                strcmp(first, "sequence,elapsed_ns,line_offset,edge,value\n") != 0) {
                fprintf(stderr, "invalid CSV header\n"); fclose(in); return 2;
            }
        }
        if (replay_csv(in, &options, replay_print, NULL) != 0) {
            fprintf(stderr, "replay failed: malformed event log or callback error\n");
            fclose(in); return 2;
        }
        if (fclose(in) != 0) { perror("close input"); return 1; }
        return 0;
    }
    usage(stderr);
    return 2;
}
