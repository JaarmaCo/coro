#include "coro.h"
#include "coro-impl.h"

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

Coro *coro_allocate(size_t min_stack_size) {

    void *addr = 0;
    size_t len = min_stack_size;
    int prot = PROT_READ | PROT_WRITE;
    int flags
        = MAP_ANONYMOUS /* Don't map to a file */
        | MAP_PRIVATE   /* Don't share with other processes */;
    int fd = -1;
    off_t offset = 0;

    size_t page_size = coro_page_size();
    if (len < page_size) {
        len = page_size;
    } else if (len % page_size != 0) {
        len += page_size - (len % page_size);
    }

    void *memory = mmap(addr, len, prot, flags, fd, offset);
    if (!memory) {
        return 0;
    }

    Coro *coro = (Coro *)memory;
    memset(coro, 0, sizeof *coro);
    
    coro->stack_size = len - sizeof *coro;
    coro->stack_begin = coro + 1;
    return coro;
}

void coro_init(Coro *coro, void (*entrypoint)(void *), void *arg) {
    assert(NULL != coro);

    coro->entrypoint = entrypoint;
    coro->arg = arg;
    coro->state = CORO_INIT;
    
    memset(&coro->reg, 0, sizeof coro->reg);
    coro->reg.rip = (uint64_t)&coro_execute_;
    coro->reg.rbp = (uint64_t)coro->stack_begin + coro->stack_size;
    coro->reg.rsp = coro->reg.rbp;
}

void coro_reset(Coro *coro) {
    
    if (!coro) {
        return;
    }

    if (coro->state == CORO_RESET) {
        return;
    }

    assert(coro->state == CORO_EXITED || coro->state == CORO_INIT);

    memset(&coro->reg, 0, sizeof coro->reg);

    coro->entrypoint = NULL;
    coro->arg = NULL;
    coro->state = CORO_RESET;
}

void coro_free(Coro *coro) {
    
    if (!coro) {
        return;
    }

    assert(coro->state == CORO_RESET);
    assert(NULL == coro->entrypoint);
    munmap(coro, coro->stack_size + sizeof *coro);
}

bool coro_swap(Coro *coro) {

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


