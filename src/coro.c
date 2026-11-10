#include "coro.h"
#include "coro-impl.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <sys/mman.h>

static _Thread_local CoroRegisters coro_root_ = {0};
static _Thread_local Coro *volatile coro_this_ = 0;

void coro_execute_(void) {
    assert(coro_this_);
    assert(coro_this_->state == CORO_ACTIVE);
    
    coro_this_->entrypoint(coro_this_->arg);

    assert(coro_this_->state == CORO_ACTIVE);
    coro_exit();
}

size_t coro_page_size(void) {
    long size = sysconf(_SC_PAGESIZE);
    if (size < 0) {
        perror("sysconf");
        exit(1);
    }
    return (size_t)size;
}

void *coro_stack_allocate_(size_t min_stack_size, size_t *mapped_size, size_t *stack_offset, size_t *stack_size) {
    
    size_t len = min_stack_size;
    size_t page_size = coro_page_size();
    if (len < page_size) {
        len = page_size;
    } else if (len % page_size != 0) {
        len += page_size - (len % page_size);
    }

    // Allocate both guard pages, and the stack
    void *full_allocation = mmap(NULL, page_size + len + page_size,
                                 PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (full_allocation == MAP_FAILED) {
        return NULL;
    }

    // Put the stack between both guard pages.
    void *guard_page_end = (void *)((uintptr_t)full_allocation + page_size);
    void *stack_page = mmap(guard_page_end, len,
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
                            -1, 0);
    if (stack_page == MAP_FAILED) {
        munmap(full_allocation, page_size);
        return NULL;
    }

    *stack_offset = page_size;
    *stack_size = len;
    *mapped_size = page_size + len + page_size;
    return full_allocation;
}

Coro *coro_allocate(size_t min_stack_size) {

    Coro *coro = (Coro *)calloc(1, sizeof *coro);
    if (!coro) {
        return NULL;
    }

    coro->stack_mapping = coro_stack_allocate_(min_stack_size,
            &coro->mapping_size, &coro->stack_offset, &coro->stack_size);

    if (!coro->stack_mapping) {
        free(coro);
        return NULL;
    }
    return coro;
}

void coro_init(Coro *coro, void (*entrypoint)(void *), void *arg) {
    assert(NULL != coro);

    coro->entrypoint = entrypoint;
    coro->arg = arg;
    coro->state = CORO_INIT;

    uint64_t stack_top = (uint64_t)coro->stack_mapping + coro->stack_offset + coro->stack_size;

    memset(&coro->reg, 0, sizeof coro->reg);
    coro->reg.rip = (uint64_t)&coro_execute_;
    coro->reg.rbp = stack_top;
    
    // leave space for a return address and align to 16 bytes
    coro->reg.rsp = (stack_top - 8) & (uint64_t)~0xf; 
}

void coro_reset(Coro *coro) {
    
    if (!coro) {
        return;
    }

    if (coro->state == CORO_RESET) {
        return;
    }

    assert(coro->state != CORO_ACTIVE);

    memset(&coro->reg, 0, sizeof coro->reg);

    coro->entrypoint = NULL;
    coro->arg = NULL;
    coro->state = CORO_RESET;
}

void coro_free(Coro *coro) {
    
    if (!coro) {
        return;
    }

    assert(coro->state == CORO_RESET || coro->state == CORO_EXITED);
    
    munmap(coro->stack_mapping, coro->mapping_size);
    
    memset(coro, 0, sizeof *coro);
    free(coro);
}

bool coro_swap(Coro *coro) {

    if (coro && coro->state == CORO_EXITED) {
        return false;
    }

    assert(!coro || coro->state == CORO_INIT || coro->state == CORO_SUSPENDED);

    Coro *restore = coro_this_;
    CoroRegisters *reg = restore ? &restore->reg : &coro_root_;

    if (coro_this_ && coro_this_->state != CORO_EXITED) {

        assert(coro_this_->state == CORO_ACTIVE);
        
        coro_this_->caller = coro;
        coro_this_->state = CORO_SUSPENDED;
    }

    coro_this_ = coro;
    {
        if (coro) {
            coro->state = CORO_ACTIVE;
        }
        coro_exchange_(reg, coro ? &coro->reg : &coro_root_);
        assert(!coro || coro->state == CORO_SUSPENDED || coro->state == CORO_EXITED);
    }
    coro_this_ = restore;

    if (coro_this_ && coro_this_->state != CORO_EXITED) {
        coro_this_->state = CORO_ACTIVE;
    }
    return !coro || coro->state != CORO_EXITED;
}

void coro_exit(void) {
    assert(NULL != coro_this_);

    Coro *caller = coro_this_->caller;
    coro_this_->caller = NULL;
    coro_this_->state = CORO_EXITED;

    coro_this_ = caller;
    if (coro_this_) {
        coro_this_->state = CORO_ACTIVE;
    }
    coro_final_exchange_(caller ? &caller->reg : &coro_root_);

    fputs("coro_exit returned.", stderr);
    exit(1);
}

void coro_yield(void) {
    assert(coro_this_);
    coro_swap(coro_this_->caller);
}

Coro *coro_this(void) {
    return coro_this_;
}

CoroState coro_state(Coro *coro) {
    if (coro) {
        return coro->state;
    }
    return coro_this_ ? CORO_SUSPENDED : CORO_ACTIVE;
}
