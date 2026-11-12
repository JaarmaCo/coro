#include "coro.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(expr) do {                                      \
    if (!(expr)) {                                             \
        fprintf(stderr, "%s:%d: CHECK failed: %s\n",           \
                __FILE__, __LINE__, #expr);                    \
        abort();                                               \
    }                                                          \
} while (0)

typedef struct TestContext {
    int counter;
    int expected_counter;
} TestContext;

static void
test_entrypoint(void *arg)
{
    TestContext *ctx = arg;

    CHECK(coro_this() != NULL);
    CHECK(coro_state(coro_this()) == CORO_ACTIVE);

    CHECK(ctx->counter == ctx->expected_counter);
    ctx->counter++;

    coro_yield();

    CHECK(coro_this() != NULL);
    CHECK(coro_state(coro_this()) == CORO_ACTIVE);

    CHECK(ctx->counter == ctx->expected_counter + 1);
    ctx->counter++;

    coro_yield();

    CHECK(coro_this() != NULL);
    CHECK(ctx->counter == ctx->expected_counter + 2);
    ctx->counter++;

    /* Returning should implicitly exit the coroutine. */
}

static void
test_page_size(void)
{
    size_t page_size = coro_page_size();

    CHECK(page_size > 0);
    CHECK((page_size & (page_size - 1)) == 0);
}

static void
test_allocate(void)
{
    Coro *coro = coro_allocate(0);

    CHECK(coro != NULL);
    CHECK(coro_state(coro) == CORO_RESET);

    CHECK(coro_this() == NULL);
    CHECK(coro_state(NULL) == CORO_ACTIVE);

    coro_free(coro);
}

static void
test_execution(void)
{
    TestContext ctx = {
        .counter = 0,
        .expected_counter = 0,
    };

    Coro *coro = coro_allocate(0);
    CHECK(coro != NULL);

    CHECK(coro_state(coro) == CORO_RESET);

    coro_init(coro, test_entrypoint, &ctx);

    CHECK(coro_state(coro) == CORO_INIT);
    CHECK(coro_this() == NULL);
    CHECK(coro_state(NULL) == CORO_ACTIVE);

    /* First execution: coroutine runs until its first yield. */

    CHECK(coro_swap(coro));
    CHECK(ctx.counter == 1);
    CHECK(coro_state(coro) == CORO_SUSPENDED);
    CHECK(coro_this() == NULL);
    CHECK(coro_state(NULL) == CORO_ACTIVE);

    /* Resume: coroutine runs until its second yield. */

    CHECK(coro_swap(coro));
    CHECK(ctx.counter == 2);
    CHECK(coro_state(coro) == CORO_SUSPENDED);

    /* Resume again: entrypoint returns and coroutine exits. */

    CHECK(!coro_swap(coro));
    CHECK(ctx.counter == 3);
    CHECK(coro_state(coro) == CORO_EXITED);

    /* An exited coroutine must not execute again. */

    CHECK(!coro_swap(coro));
    CHECK(ctx.counter == 3);
    CHECK(coro_state(coro) == CORO_EXITED);

    coro_free(coro);
}

static void
exit_entrypoint(void *arg)
{
    int *counter = arg;

    (*counter)++;
    coro_exit();

    /* Unreachable. */
}

static void
test_explicit_exit(void)
{
    int counter = 0;

    Coro *coro = coro_allocate(0);
    CHECK(coro != NULL);

    coro_init(coro, exit_entrypoint, &counter);

    CHECK(coro_swap(coro) == false);
    CHECK(counter == 1);
    CHECK(coro_state(coro) == CORO_EXITED);

    coro_free(coro);
}

static void
test_reset_and_reuse(void)
{
    TestContext ctx = {
        .counter = 0,
        .expected_counter = 0,
    };

    Coro *coro = coro_allocate(0);
    CHECK(coro != NULL);

    coro_init(coro, test_entrypoint, &ctx);

    CHECK(coro_swap(coro));
    CHECK(ctx.counter == 1);
    CHECK(coro_state(coro) == CORO_SUSPENDED);

    coro_reset(coro);

    CHECK(coro_state(coro) == CORO_RESET);

    /* Reinitialize the same coroutine with a fresh context. */

    ctx.counter = 0;

    coro_init(coro, test_entrypoint, &ctx);

    CHECK(coro_state(coro) == CORO_INIT);

    CHECK(coro_swap(coro));
    CHECK(ctx.counter == 1);

    CHECK(coro_swap(coro));
    CHECK(ctx.counter == 2);

    CHECK(!coro_swap(coro));
    CHECK(ctx.counter == 3);
    CHECK(coro_state(coro) == CORO_EXITED);

    coro_reset(coro);
    CHECK(coro_state(coro) == CORO_RESET);

    coro_free(coro);
}

typedef struct SequenceContext {
    int *sequence;
    int *position;
    int id;
} SequenceContext;

static void
sequence_entrypoint(void *arg)
{
    SequenceContext *ctx = arg;

    ctx->sequence[(*ctx->position)++] = ctx->id;
    coro_yield();

    ctx->sequence[(*ctx->position)++] = ctx->id;
    coro_yield();

    ctx->sequence[(*ctx->position)++] = ctx->id;
}

static void
test_multiple_coroutines(void)
{
    int sequence[6] = {0};
    int position = 0;

    SequenceContext a = {
        .sequence = sequence,
        .position = &position,
        .id = 1,
    };

    SequenceContext b = {
        .sequence = sequence,
        .position = &position,
        .id = 2,
    };

    Coro *coro_a = coro_allocate(0);
    Coro *coro_b = coro_allocate(0);

    CHECK(coro_a != NULL);
    CHECK(coro_b != NULL);

    coro_init(coro_a, sequence_entrypoint, &a);
    coro_init(coro_b, sequence_entrypoint, &b);

    CHECK(coro_swap(coro_a));
    CHECK(coro_swap(coro_b));

    CHECK(coro_swap(coro_a));
    CHECK(coro_swap(coro_b));

    CHECK(!coro_swap(coro_a));
    CHECK(!coro_swap(coro_b));

    CHECK(position == 6);

    const int expected[] = {1, 2, 1, 2, 1, 2};

    for (int i = 0; i < 6; i++) {
        CHECK(sequence[i] == expected[i]);
    }

    CHECK(coro_state(coro_a) == CORO_EXITED);
    CHECK(coro_state(coro_b) == CORO_EXITED);

    coro_free(coro_a);
    coro_free(coro_b);
}

int
main(void)
{
    test_page_size();
    test_allocate();
    test_execution();
    test_explicit_exit();
    test_reset_and_reuse();
    test_multiple_coroutines();

    puts("All coroutine tests passed.");

    return 0;
}
