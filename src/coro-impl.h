#ifndef CORO_IMPL_H_

#include <stdint.h>

#include "coro.h"

typedef struct CoroRegisters CoroRegisters;

struct CoroRegisters {

    uint64_t rip; // instruction pointer
    uint64_t rsp; // stack pointer
    uint64_t rbp; // base pointer
    
    // saved registers
    uint64_t rbx;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
};

struct Coro {

    CoroRegisters reg;

    CoroState state;
    Coro *caller;

    void *arg;
    void(*entrypoint)(void *);

    void *stack_mapping;
    size_t mapping_size;
    size_t stack_offset;
    size_t stack_size;
};

// Make new_context active and store a restorable context to old_context
void
coro_exchange_(CoroRegisters *old_context, CoroRegisters *new_context);

// Restore context, discarding the active context
void
coro_final_exchange_(CoroRegisters *context);

#define CORO_IMPL_H_
#endif
