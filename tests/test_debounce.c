#include "debounce.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    debounce_state_t s;
    int accepted = -1;
    debounce_init(&s, 25000000ULL, 1);
    assert(!debounce_update(&s, 0, 100000000ULL, &accepted));
    assert(!debounce_update(&s, 1, 103000000ULL, &accepted));
    assert(!debounce_update(&s, 0, 109000000ULL, &accepted));
    assert(!debounce_update(&s, 0, 133000000ULL, &accepted));
    assert(debounce_update(&s, 0, 134000000ULL, &accepted));
    assert(accepted == 0);
    assert(!debounce_update(&s, 2, 200000000ULL, &accepted));
    puts("test_debounce: PASS");
    return 0;
}
