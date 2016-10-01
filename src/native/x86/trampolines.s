.section .text

# Send trampoline
.global _slvm_dynrun_send_trap
.global slvm_dynrun_send
_slvm_dynrun_send_trap:
    push %esp
    call slvm_dynrun_send
    add $4, %esp
    ret

.global _slvm_dynrun_returnToSend_trap
_slvm_dynrun_returnToSend_trap:
    movl 8(%esp), %eax
    movl 4(%esp), %esp
    ret
