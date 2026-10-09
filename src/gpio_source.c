#define _POSIX_C_SOURCE 200809L
#include "gpio_source.h"
#include "debounce.h"



int gpio_source_demo(FILE *output, const demo_source_options_t *options)
{
    static const struct { uint64_t at_ms; int value; } samples[] = {
        {0, 1}, {100, 0}, {103, 1}, {109, 0}, {160, 0},
        {300, 1}, {303, 0}, {360, 1}, {700, 0}, {730, 0}
    };
    debounce_state_t debounce;
    uint64_t duration_ns;
    uint64_t sequence = 0;
    int accepted = 1;
    if (output == NULL || options == NULL) return -1;
    duration_ns = options->duration_ms * 1000000ULL;
    debounce_init(&debounce, options->debounce_ms * 1000000ULL, 1);
    if (event_write_csv_header(output) != 0) return -1;
    for (unsigned int i = 0; i < sizeof(samples)/sizeof(samples[0]); ++i) {
        uint64_t elapsed = samples[i].at_ms * 1000000ULL;
        if (elapsed > duration_ns && options->duration_ms != 0) break;
        if (i == 0) continue;
        {
            int new_value;
            if (debounce_update(&debounce, samples[i].value, elapsed, &new_value)) {
                gpio_event_t e = {
                    .sequence = sequence++,
                    .elapsed_ns = elapsed,
                    .line_offset = options->line_offset,
                    .edge = new_value ? EVENT_EDGE_RISING : EVENT_EDGE_FALLING,
                    .value = new_value
                };
                if (event_write_csv(output, &e) != 0) return -1;
                if (options->verbose)
                    (void)fprintf(stderr, "accepted line=%u edge=%s at %.3f ms\n",
                                  e.line_offset, event_edge_name(e.edge),
                                  (double)e.elapsed_ns / 1.0e6);
                accepted++;
            }
        }
    }
    return accepted - 1;
}
