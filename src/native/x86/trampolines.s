.section .text

# Send trampoline
.global _slvm_dynrun_send_trampoline
.global slvm_dynrun_send_dispatch
_slvm_dynrun_send_trampoline:
    push %ebp
    movl %esp, %ebp

    push %eax # Selector
    push %ecx # Argument description

    # Store the stack pointer
    movl %esp, %eax

    # Fetch the stack segment pointer
    movl %esp, %ebx
    andl $-4096, %ebx
    addl $4096, %ebx

    # Fetch the thread data
    movl -24(%ebx), %ecx

    # Store the smalltalk stack pointers.
    movl %esp, -12(%ebx)
    movl %ebp, -16(%ebx)

    # Restore the C stack
    movl 0(%ecx), %esp
    movl 4(%ecx), %ebp

    # Pass the smalltalk stack pointer
    push %eax
    push $0
    call slvm_dynrun_send_dispatch
    hlt


# _slvm_dynrun_stack_limit_trap
.global _slvm_dynrun_stack_limit_trap
.global slvm_dynrun_new_stack_segment
_slvm_dynrun_stack_limit_trap:
    # Fetch the stack segment pointer
    movl %esp, %ebx
    andl $-4096, %ebx
    addl $4096, %ebx

    # Fetch the thread data
    movl -24(%ebx), %ecx

    # Store the smalltalk stack pointers.
    movl %esp, -12(%ebx)
    movl %ebp, -16(%ebx)

    # Restore the C stack
    movl 0(%ecx), %esp
    movl 4(%ecx), %ebp

    # Pass some arguments
    pushl %eax
    pushl %ebx

    # Call C function for creating a new stack segment.
    call slvm_dynrun_new_stack_segment
    hlt

.global _slvm_dynrun_pop_stack_segment
.global _slvm_dynrun_pop_stack_segment_trap
_slvm_dynrun_pop_stack_segment_trap:
    # Fetch the stack segment pointer
    movl %esp, %ebx
    andl $-4096, %ebx
    addl $4096, %ebx

    # Fetch the thread data
    movl -24(%ebx), %ecx

    # Store the smalltalk stack pointers.
    movl %esp, -12(%ebx)
    movl %ebp, -16(%ebx)

    # Restore the C stack
    movl 0(%ecx), %esp
    movl 4(%ecx), %ebp

    # Pass some arguments
    pushl %eax
    pushl %ebx
    call slvm_dynrun_pop_stack_segment

.global slvm_dynrun_csend
slvm_dynrun_csend:
    pushl %ebp
    movl %esp, %ebp

    push %esp
    push $1
    call slvm_dynrun_send_dispatch
    addl $8, %esp

    movl %ebp, %esp
    popl %ebp
    ret

# (4) restoreStackPointer, (8)result
.global _slvm_dynrun_returnToSender_trampoline
_slvm_dynrun_returnToSender_trampoline:
    # Store the result in EAX
    movl 8(%esp), %eax

    # Restore the stack frame.
    movl 4(%esp), %esp
    popl %ebp

    # Return
    ret

# (4) restoreStackPointer, (8)methodPointer
.global _slvm_dynrun_returnToMethod_trampoline
_slvm_dynrun_returnToMethod_trampoline:
    # Store the method pointer in EAX
    movl 8(%esp), %eax

    # Restore the stack frame.
    movl 4(%esp), %esp
    popl %ebp

    # Jump to the method
    jmp *%eax

.global _slvm_dynrun_switch_to_smalltalk

# Stack segment header
# (-8) header;
# (-12) stackPointer;
# (-16) framePointer;
# (-20) linkPointer;
# (-24) threadData;
# (-28) segmentSize;
# (-32) reserved;

# (8) stackSegment (12) entryPoint
_slvm_dynrun_switch_to_smalltalk:
# (SLVM_ExecutionStackSegmentHeader *stackSegment, SLVM_ThreadStackData *threadData, void *entryPoint);
    pushl %ebp
    movl %esp, %ebp

    pushl %ebx
    pushl %edi
    pushl %esi

    movl 8(%ebp), %ebx
    movl 12(%ebp), %eax

    # Fetch the thread data pointer
    movl -24(%ebx), %ecx

    # Store the old C pointers
    pushl 8(%ecx)
    pushl 4(%ecx)
    pushl 0(%ecx)

    # Store the C pointers
    movl %esp, 0(%ecx)
    movl %ebp, 4(%ecx)
    movl $.LbackToCReturnPointer, 8(%ecx)

    # Switch to the smalltalk stack.
    movl -12(%ebx), %esp
    movl -16(%ebx), %ebp

    # Jump to the function
    jmp *%eax

.LbackToCReturnPointer:
    # Restore the old C pointers
    popl 0(%ecx)
    popl 4(%ecx)
    popl 8(%ecx)

    # Restore the saved registers and return
    popl %esi
    popl %edi
    popl %ebx

    movl %ebp, %esp
    popl %ebp
    ret

.global _slvm_returnToC_trampoline
_slvm_returnToC_trampoline:

    # Fetch the stack segment pointer
    movl %esp, %ebx
    andl $-4096, %ebx
    addl $4096, %ebx

    # Fetch the thread data
    movl -24(%ebx), %ecx

    # Store the smalltalk stack pointers.
    movl %esp, -12(%ebx)
    movl %ebp, -16(%ebx)

    # Restore the C stack
    movl 0(%ecx), %esp
    movl 4(%ecx), %ebp

    # Return to C
    pushl 8(%ecx)
    ret
