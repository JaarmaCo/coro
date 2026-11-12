#ifndef CORO_IMPL_H_

#include <stdint.h>

#include "coro.h"

typedef struct CoroRegisters CoroRegisters;

struct CoroRegisters {

    uintptr_t rip; // instruction pointer
    uintptr_t rsp; // stack pointer
    uintptr_t rbp; // base pointer
    
    // saved registers
    uintptr_t rbx;
    uintptr_t r12;
    uintptr_t r13;
    uintptr_t r14;
    uintptr_t r15;
    uintptr_t rdi;
    uintptr_t rsi;
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
coro_exchange_(CoroRegisters *old_context, const CoroRegisters *new_context);

// Restore context, discarding the active context
void
coro_final_exchange_(CoroRegisters *context);

#define CORO_IMPL_H_
#endif
