# Coro

This is a minimal C coroutine implementation targeting Linux (x64).

Example usage:

```c
#include <coro.h>
#include <stdio.h>
#include <stdlib.h>

struct Counter {
    int count;
    int step;
    int value;
};

static void run_counter(void *arg) {
    struct Counter *state = (struct Counter *)arg;

    for (int i = 0; i < state->count; ++i) {
        coro_yield();
        state->value += state->step;
    }
}

int main(void) {

    struct Counter counter = {
        .count = 10,
        .step = 2,
        .value = 0,
    };

    Coro *coro = coro_allocate(0);
    if (!coro) {
        perror("coro_allocate");
        exit(1);
    }
    coro_init(coro, &run_counter, &counter);
    while (coro_swap(coro)) {
        printf("%d\n", counter.value);
    }
    coro_free(coro);
}
```

Output:

```
0
2
6
8
10
12
14
16
18
```

