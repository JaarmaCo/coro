#include "coro.h"

#include <stdlib.h>
#include <stdio.h>

void runner(void *arg) {
    
    int *x = (int *)arg;
    while (*x < 10) {
        coro_yield();
        ++*x;
    }
}

int main(void) {

    Coro *coro = coro_allocate(coro_page_size());
    if (!coro) {
        perror("coro_allocate");
        exit(1);
    }

    int arg = 0;
    coro_init(coro, &runner, &arg);

    for (int i = 0; coro_swap(coro); ++i) {
        printf("arg[%d] = %d\n", i, arg);
    }

    coro_reset(coro);
    coro_free(coro);
}

#include "coro.c"

