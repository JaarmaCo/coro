#include "coro-impl.h"

__attribute__((naked, sysv_abi))
void coro_exchange_(CoroRegisters *old_context __attribute__((unused)),
        const CoroRegisters *new_context __attribute__((unused))) {

    // Save return address to old_context->rip
    __asm__ volatile (
        // Put args in scratch registers
        "mov 4(%esp), %ecx\n\t" // old_context => ecx
        "mov 8(%esp), %edx\n\t" // new_context => edx

        // Record return address
        "mov 0(%esp), %eax\n\t"
        "mov %eax, 0(%ecx)\n\t"

        // Record SP
        "lea 12(%esp), %eax\n\t" // 4 (ret) + 4 (arg1) + 4 (arg2)
        "mov %eax, 4(%ecx)\n\t"
       
        // Record non-volatile
        "mov %ebp, 8(%ecx)\n\t"
        "mov %ebx, 12(%ecx)\n\t"
        
        // Restore state
        "mov 8(%edx), %ebp\n\t"
        "mov 12(%edx), %ebx\n\t"
        "mov 4(%edx), %esp\n\t"
        
        // Execute
        "jmp *0(%edx)"
    );
}

