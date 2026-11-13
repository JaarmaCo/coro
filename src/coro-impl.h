#ifndef CORO_IMPL_H_

#include <stdint.h>

#include "coro.h"

typedef struct CoroRegisters CoroRegisters;

#ifdef __x86_64__

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

#elif defined(__aarch64__)

struct CoroRegisters {

    uintptr_t lr; // link register (return address)
    uintptr_t sp; // stack pointer

    uintptr_t x19;
    uintptr_t x20;
    uintptr_t x21;
    uintptr_t x22;
    uintptr_t x23;
    uintptr_t x24;
    uintptr_t x25;
    uintptr_t x26;
    uintptr_t x27;
    uintptr_t x28;
    uintptr_t x29;
};

#endif

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
