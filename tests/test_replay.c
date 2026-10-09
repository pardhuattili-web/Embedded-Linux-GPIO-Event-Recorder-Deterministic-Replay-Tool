#include "replay.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct { int count; uint64_t last_elapsed; uint64_t first_sequence; bool have_first; } test_context_t;
static int collect(const gpio_event_t *e, void *ctx)
{
    test_context_t *t = ctx;
    if (!t->have_first) { t->first_sequence = e->sequence; t->have_first = true; }
    assert(e->sequence == t->first_sequence + (uint64_t)t->count);
    assert(e->elapsed_ns >= t->last_elapsed);
    t->last_elapsed = e->elapsed_ns;
    t->count++;
    return 0;
}

int main(void)
{
    FILE *f = tmpfile();
    test_context_t ctx = {0, 0, 0, false};
    replay_options_t opts = {.speed = 1.0, .no_sleep = true, .verbose = false};
    assert(f != NULL);
    fputs("0,0,17,rising,1\n1,1000,17,falling,0\n", f);
    rewind(f);
    assert(replay_csv(f, &opts, collect, &ctx) == 0);
    assert(ctx.count == 2 && ctx.last_elapsed == 1000);
    fclose(f);
    {
        FILE *bad = tmpfile();
        assert(bad != NULL);
        fputs("1,0,17,rising,1\n0,20,17,falling,0\n", bad);
        rewind(bad);
        ctx = (test_context_t){0, 0, 0, false};
        assert(replay_csv(bad, &opts, collect, &ctx) == -1);
        fclose(bad);
    }
    puts("test_replay: PASS");
    return 0;
}
