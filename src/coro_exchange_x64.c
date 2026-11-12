#include "coro-impl.h"

__attribute__((naked))
void coro_exchange_(CoroRegisters *old_context __attribute__((unused)),
        const CoroRegisters *new_context __attribute__((unused))) {
    
    // Save return address to old_context->rip
    __asm__("movq 0(%rsp), %rax "); // %rax = stack_pointer[RET_OFFSET]
    __asm__("movq %rax, 0(%rdi) "); // old_context->rip = %rax

    // Save stack pointer to old_context->rsp
    __asm__("movq 8(%rsp), %rax "); // %rax = stack_pointer[SP_OFFSET]
    __asm__("movq %rax,  8(%rdi)"); // old_context->rsp = %rax
   
    // Save other sters
    __asm__("movq %rbp, 16(%rdi)"); // old_context->rbp = %rbp
    __asm__("movq %rbx, 24(%rdi)"); // old_context->rbx = %rbx
    __asm__("movq %r12, 32(%rdi)"); // old_context->r12 = %r12
    __asm__("movq %r13, 40(%rdi)"); // old_context->r13 = %r13
    __asm__("movq %r14, 48(%rdi)"); // old_context->r14 = %r14
    __asm__("movq %r15, 56(%rdi)"); // old_context->r15 = %r15
    
    // Restore the activated context
    __asm__("movq 16(%rsi), %rbp"); // %rbp = new_context->rbp
    __asm__("movq 24(%rsi), %rbx"); // %rbx = new_context->rbx
    __asm__("movq 32(%rsi), %r12"); // %r12 = new_context->r12
    __asm__("movq 40(%rsi), %r13"); // %r13 = new_context->r13
    __asm__("movq 48(%rsi), %r14"); // %r14 = new_context->r14
    __asm__("movq 56(%rsi), %r15"); // %r15 = new_context->r15
   
    // Restore the stack pointer
    __asm__("movq 8(%rsi), %rsp"); // %rsp = new_context->rsp
    
    // Execute the context
    __asm__("jmp *0(%rsi)      "); // %rip = new_context->rip
}
