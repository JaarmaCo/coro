.global coro_exchange_

# void coro_exchange_(CoroRegisters *old_context /* %rdi */, CoroRegisters *new_context /* %rsi */)
coro_exchange_:
    
    # Save return address to old_context->rip
    movq 0(%rsp), %rax  # %rax = stack_pointer[RET_OFFSET]
    movq %rax, 0(%rdi)  # old_context->rip = %rax

    # Save stack pointer to old_context->rsp
    movq 8(%rsp), %rax  # %rax = stack_pointer[SP_OFFSET]
    movq %rax,  8(%rdi) # old_context->rsp = %rax
   
    # Save other sters
    movq %rbp, 16(%rdi) # old_context->rbp = %rbp
    movq %rbx, 24(%rdi) # old_context->rbx = %rbx
    movq %r12, 32(%rdi) # old_context->r12 = %r12
    movq %r13, 40(%rdi) # old_context->r13 = %r13
    movq %r14, 48(%rdi) # old_context->r14 = %r14
    movq %r15, 56(%rdi) # old_context->r15 = %r15
    
    # Restore the activated context
    movq 16(%rsi), %rbp # %rbp = new_context->rbp
    movq 24(%rsi), %rbx # %rbx = new_context->rbx
    movq 32(%rsi), %r12 # %r12 = new_context->r12
    movq 40(%rsi), %r13 # %r13 = new_context->r13
    movq 48(%rsi), %r14 # %r14 = new_context->r14
    movq 56(%rsi), %r15 # %r15 = new_context->r15
   
    # Restore the stack pointer
    movq 8(%rsi), %rsp # %rsp = new_context->rsp
    
    # Execute the context
    jmp *0(%rsi)       # %rip = new_context->rip

