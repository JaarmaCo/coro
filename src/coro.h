#ifndef CORO_H_

#include <stdbool.h>
#include <stddef.h>

#if defined(__x86_64__)
#   if __SIZEOF_POINTER__ == 8
#       define CORO_PLATFORM_x64
#   elif __SIZEOF_POINTER__ == 4
#       define CORO_PLATFORM_x86
#   else
#       error "Platform size not supported"
#   endif
#elif defined(__aarch64__)
#   if __SIZEOF_POINTER__ == 8
#       define CORO_PLATFORM_arm64
#   else
#       error "Platform not supported"
#   endif
#else
#   error "Platform not supported"
#endif

typedef struct Coro Coro;

typedef enum CoroState {
    CORO_RESET,
    CORO_INIT,
    CORO_ACTIVE,
    CORO_SUSPENDED,
    CORO_EXITED,
} CoroState;

size_t
coro_page_size(void);

Coro *
coro_allocate(size_t min_stack_size);

void
coro_init(Coro *coro, void(*entrypoint)(void *), void *arg);

void
coro_reset(Coro *coro);

void
coro_free(Coro *coro);

_Noreturn void
coro_exit(void);

bool
coro_swap(Coro *coro);

void
coro_yield(void);

Coro *
coro_this(void);

CoroState
coro_state(Coro *coro);

#define CORO_H_
#endif
