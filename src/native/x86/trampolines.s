.section .text

# Send trampoline
.global _slvm_dynrun_send_trap
.global slvm_dynrun_send_dispatch
_slvm_dynrun_send_trap:
    push %ebp
    movl %esp, %ebp

    push %esp
    push $0
    call slvm_dynrun_send_dispatch

    movl %ebp, %esp
    popl %ebp
    ret

.global slvm_dynrun_csend
slvm_dynrun_csend:
    push %ebp
    movl %esp, %ebp

    push %esp
    push $1
    call slvm_dynrun_send_dispatch

    movl %ebp, %esp
    popl %ebp
    ret

.global _slvm_dynrun_returnToSender_trap
_slvm_dynrun_returnToSender_trap:
    movl 8(%esp), %eax
    movl 4(%esp), %esp
    popl %ebp
    ret

# (4) restoreStackPointer, (8)methodPointer
.global _slvm_dynrun_returnToMethod_trap
_slvm_dynrun_returnToMethod_trap:
    # Store the method pointer in EAX
    movl 8(%esp), %eax

    # Restore the stack frame.
    movl 4(%esp), %esp
    popl %ebp

    # Put the method pointer below the return pointer position.
    # Argument description point.
    movl %eax, 4(%esp)

    # Move the return pointer to where the selector is located.
    popl %eax
    movl %eax, 4(%esp)

    # Return to the method.
    ret

.global _slvm_dynrun_smalltalk_sendWithArguments
# (8) entryPoint, (12) argumentDescription [12 oopCount, 14 nativeCount], (16)SLVM_Oop receiver, (20) oopArguments, (24)nativeArguments
_slvm_dynrun_smalltalk_sendWithArguments:
    pushl %ebp
    movl %esp, %ebp
    pushl %esi
    pushl %edi
    pushl %ecx

    # Align the stack.
    andl $-16, %esp

    # Reserve the native space
    movzxw 14(%ebp), %ecx
    subl %ecx, %esp

    # Copy the native arguments.
    cld
    shrl $2, %ecx
    movl 24(%ebp), %esi
    movl %esp, %edi
    rep movsl

    # Reserve the oop space
    movzxw 12(%ebp), %ecx
    movl %ecx, %eax

    #andl $-16, %esp
    shll $2, %eax
    subl %eax, %esp

    # Copy the oop arguments
    cld
    movl 20(%ebp), %esi
    movl %esp, %edi
    rep movsl

    # Push the receiver
    pushl 16(%ebp)

    # Call the method.
    movl 8(%ebp), %eax
    call *%eax

    movl %ebp, %esp
    sub $12, %esp
    popl %ecx
    popl %esi
    popl %edi
    popl %ebp
    ret
