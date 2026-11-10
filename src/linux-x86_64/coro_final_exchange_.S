.global coro_final_exchange_

# void coro_final_exchange_(CoroRegisters *context /* %rdi */);
coro_final_exchange_:
     
    # Restore the activated context
    movq 16(%rdi), %rbp # %rbp = context->rbp
    movq 24(%rdi), %rbx # %rbx = context->rbx
    movq 32(%rdi), %r12 # %r12 = context->r12
    movq 40(%rdi), %r13 # %r13 = context->r13
    movq 48(%rdi), %r14 # %r14 = context->r14
    movq 56(%rdi), %r15 # %r15 = context->r15
   
    # Restore the stack pointer
    movq 8(%rdi), %rsp # %rsp = context->rsp
    
    # Execute the context
    jmp *0(%rdi)       # %rip = context->rip


