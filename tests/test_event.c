#define _POSIX_C_SOURCE 200809L
#include "event.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    gpio_event_t in = {4, 123456789, 17, EVENT_EDGE_FALLING, 0};
    gpio_event_t out = {0};
    FILE *f = tmpfile();
    assert(f != NULL);
    assert(event_write_csv_header(f) == 0);
    assert(event_write_csv(f, &in) == 0);
    rewind(f);
    { char header[128]; assert(fgets(header, sizeof(header), f) != NULL); }
    assert(event_read_csv(f, &out) == 1);
    assert(out.sequence == in.sequence);
    assert(out.elapsed_ns == in.elapsed_ns);
    assert(out.line_offset == in.line_offset);
    assert(out.edge == in.edge && out.value == in.value);
    assert(event_read_csv(f, &out) == 0);
    fclose(f);
    {
        FILE *bad = tmpfile();
        assert(bad != NULL);
        fputs("1,10,3,rising,0\n", bad);
        rewind(bad);
        assert(event_read_csv(bad, &out) == -1);
        fclose(bad);
    }
    puts("test_event: PASS");
    return 0;
}
