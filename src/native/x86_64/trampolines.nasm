SECTION  .text

%ifdef _WIN32
%define ADD_UNDERSCORE
%endif

%ifdef _APPLE
%define ADD_UNDERSCORE
%endif

%ifdef ADD_UNDERSCORE
%define _C(symbol) _ %+ symbol
%else
%define _C(symbol) symbol
%endif

; Send trampoline
GLOBAL _C(_slvm_dynrun_send_trampoline)
EXTERN _C(slvm_dynrun_send_dispatch)
_C(_slvm_dynrun_send_trampoline):
    push  rbp
    mov  rbp,rsp

    push  rax  ; Selector
    push  rcx  ; Argument description

    ; Store the stack pointer
    mov  rax,rsp

    ; Fetch the stack segment pointer
    mov  rbx,rsp
    and  rbx,-4096
    add  rbx,4096

    ; Fetch the thread data
    mov  rcx, [rbx-48]

    ; Store the smalltalk stack pointers.
    mov  [rbx-24],rsp
    mov  [rbx-32],rbp

    ; Restore the C stack
    mov  rsp, [rcx+0]
    mov  rbp, [rcx+8]

    ; Pass the smalltalk stack pointer
    mov rdi, 0
    mov rsi, rax
    call _C(slvm_dynrun_send_dispatch)
    hlt

; _slvm_dynrun_stack_limit_trap
GLOBAL _C(_slvm_dynrun_stack_limit_trap)
EXTERN _C(slvm_dynrun_new_stack_segment)
_C(_slvm_dynrun_stack_limit_trap):

    ; Fetch the stack segment pointer
    mov  rbx,rsp
    and  rbx,-4096
    add  rbx,4096

    ; Fetch the thread data
    mov  rcx, [rbx-48]

    ; Store the smalltalk stack pointers.
    mov  [rbx-24],rsp
    mov  [rbx-32],rbp

    ; Restore the C stack
    mov  rsp, [rcx+0]
    mov  rbp, [rcx+8]

    ; Pass some arguments
    mov rdi, rbx
    mov rsi, rax

    ; Call C function for creating a new stack segment.
    call _C(slvm_dynrun_new_stack_segment)
    hlt

EXTERN _C(slvm_dynrun_pop_stack_segment)
GLOBAL _C(_slvm_dynrun_pop_stack_segment_trap)
_C(_slvm_dynrun_pop_stack_segment_trap):
    ; Fetch the stack segment pointer
    mov  rbx,rsp
    and  rbx,-4096
    add  rbx,4096

    ; Fetch the thread data
    mov  rcx, [rbx-48]

    ; Store the smalltalk stack pointers.
    mov  [rbx-24],rsp
    mov  [rbx-32],rbp

    ; Restore the C stack
    mov  rsp, [rcx+0]
    mov  rbp, [rcx+8]

    ; Pass some arguments
    mov   rdi, rbx
    mov   rsi, rax
    call _C(slvm_dynrun_pop_stack_segment)

GLOBAL _C(slvm_dynrun_csend)
_C(slvm_dynrun_csend):
    push rbp
    mov  rbp,rsp

    mov rdi, 1
    mov rsi, rsp
    call _C(slvm_dynrun_send_dispatch)

    mov  rsp, rbp
    pop  rbp
    ret

; (4) restoreStackPointer, (8)result
GLOBAL _C(_slvm_dynrun_returnToSender_trampoline)
_C(_slvm_dynrun_returnToSender_trampoline):
    ; Store the result in EAX
    mov  rax, rsi

    ; Restore the stack frame.
    mov  rsp, rdi
    pop  rbp

    ; Return
    ret

; (4) restoreStackPointer, (8)methodPointer
GLOBAL _C(_slvm_dynrun_returnToMethod_trampoline)
_C(_slvm_dynrun_returnToMethod_trampoline):
    ; Restore the stack frame.
    mov  rsp, rdi
    pop  rbp

    ; Jump to the method
    jmp rsi

GLOBAL _C(_slvm_dynrun_switch_to_smalltalk)

; Stack segment header
; (-16) header;
; (-24) stackPointer;
; (-32) framePointer;
; (-40) linkPointer;
; (-48) threadData;
; (-56) segmentSize;
; (-64) reserved;

; (RDI) stackSegment (RSI) entryPoint
_C(_slvm_dynrun_switch_to_smalltalk):
; (SLVM_ExecutionStackSegmentHeader *stackSegment(RDI), void *entryPoint(RSI));
    push   rbp
    mov  rbp,rsp

    ; Fetch the thread data pointer
    mov  rcx, [rdi-48]

    ; Store the old C pointers
    push   qword [rcx+16]
    push   qword [rcx+8]
    push   qword [rcx+0]

    ; Store the C pointers
    mov  [rcx + 0],rsp
    mov  [rcx + 8],rbp
    mov  qword [rcx + 16], .LbackToCReturnPointer

    ; Switch to the smalltalk stack.
    mov  rsp, [rdi-24]
    mov  rbp, [rdi-32]

    ; Jump to the function
    jmp rsi

.LbackToCReturnPointer:
    ; Restore the old C pointers
    pop  qword [rcx+0]
    pop  qword [rcx+8]
    pop  qword [rcx+16]

    mov  rsp,rbp
    pop  rbp
    ret

GLOBAL _C(_slvm_returnToC_trampoline)
_C(_slvm_returnToC_trampoline):
    ; Fetch the stack segment pointer
    mov  rbx,rsp
    and  rbx,-4096
    add  rbx,4096

    ; Fetch the thread data
    mov  rcx, [rbx-48]

    ; Store the smalltalk stack pointers.
    mov  [rbx-24],rsp
    mov  [rbx-32],rbp

    ; Restore the C stack
    mov  rsp, [rcx+0]
    mov  rbp, [rcx+8]

    ; Return to C
    jmp [rcx+16]

; Primitives trampolines
GLOBAL _C(slvm_dynrun_primitive_dispatch)
GLOBAL _C(slvm_dynrun_primitive_dispatch_trampoline)
_C(slvm_dynrun_primitive_dispatch_trampoline):
    push  rbp
    mov  rbp,rsp

    ; Store the stack pointer
    mov  rdi,rsp

    ; Fetch the stack segment pointer
    mov  rbx,rsp
    and  rbx,-4096
    add  rbx,4096

    ; Fetch the thread data
    mov  rcx, [rbx-48]

    ; Store the smalltalk stack pointers.
    mov  [rbx-24],rsp
    mov  [rbx-32],rbp

    ; Restore the C stack
    mov  rsp, [rcx+0]
    mov  rbp, [rcx+8]

    ; Call the primitive
    call rax
    hlt
