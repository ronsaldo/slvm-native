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
    push  ebp
    mov  ebp,esp

    push  eax  ; Selector
    push  ecx  ; Argument description

    ; Store the stack pointer
    mov  eax,esp

    ; Fetch the stack segment pointer
    mov  ebx,esp
    and  ebx,-4096
    add  ebx,4096

    ; Fetch the thread data
    mov  ecx, [ebx-24]

    ; Store the smalltalk stack pointers.
    mov  [ebx-12],esp
    mov  [ebx-16],ebp

    ; Restore the C stack
    mov  esp, [ecx+0]
    mov  ebp, [ecx+4]

    ; Pass the smalltalk stack pointer
    push  eax
    push  dword 0
    call _C(slvm_dynrun_send_dispatch)
    hlt

GLOBAL _C(_slvm_dynrun_supersend_trampoline)
EXTERN _C(slvm_dynrun_supersend_dispatch)
_C(_slvm_dynrun_supersend_trampoline):
    push  ebp
    mov  ebp,esp

    push  eax  ; Selector
    push  ecx  ; Argument description

    ; Store the stack pointer
    mov  eax,esp

    ; Fetch the stack segment pointer
    mov  ebx,esp
    and  ebx,-4096
    add  ebx,4096

    ; Fetch the thread data
    mov  ecx, [ebx-24]

    ; Store the smalltalk stack pointers.
    mov  [ebx-12],esp
    mov  [ebx-16],ebp

    ; Restore the C stack
    mov  esp, [ecx+0]
    mov  ebp, [ecx+4]

    ; Pass the smalltalk stack pointer
    push  edx
    push  eax
    push  dword 0
    call _C(slvm_dynrun_supersend_dispatch)
    hlt

; _slvm_dynrun_stack_limit_trap
GLOBAL _C(_slvm_dynrun_stack_limit_trap)
EXTERN _C(slvm_dynrun_new_stack_segment)
_C(_slvm_dynrun_stack_limit_trap):
    ; Fetch the stack segment pointer
    mov  ebx,esp
    and  ebx,-4096
    add  ebx,4096

    ; Fetch the thread data
    mov  ecx, [ebx-24]

    ; Store the smalltalk stack pointers.
    mov  [ebx-12],esp
    mov  [ebx-16],ebp

    ; Restore the C stack
    mov  esp, [ecx+0]
    mov  ebp, [ecx+4]

    ; Pass some arguments
    push   eax
    push   ebx

    ; Call C function for creating a new stack segment.
    call _C(slvm_dynrun_new_stack_segment)
    hlt

EXTERN _C(slvm_dynrun_pop_stack_segment)
GLOBAL _C(_slvm_dynrun_pop_stack_segment_trap)
_C(_slvm_dynrun_pop_stack_segment_trap):
    ; Fetch the stack segment pointer
    mov  ebx,esp
    and  ebx,-4096
    add  ebx,4096

    ; Fetch the thread data
    mov  ecx, [ebx-24]

    ; Store the smalltalk stack pointers.
    mov  [ebx-12],esp
    mov  [ebx-16],ebp

    ; Restore the C stack
    mov  esp, [ecx+0]
    mov  ebp, [ecx+4]

    ; Pass some arguments
    push   eax
    push   ebx
    call _C(slvm_dynrun_pop_stack_segment)

GLOBAL _C(slvm_dynrun_csend)
_C(slvm_dynrun_csend):
    push   ebp
    mov  ebp,esp

    push  esp
    push  dword 1
    call _C(slvm_dynrun_send_dispatch)
    add  esp,8

    mov  esp,ebp
    pop  ebp
    ret

; (4) restoreStackPointer, (8)result
GLOBAL _C(_slvm_dynrun_returnToSender_trampoline)
_C(_slvm_dynrun_returnToSender_trampoline):
    ; Store the result in EAX
    mov  eax, [esp+8]

    ; Restore the stack frame.
    mov  esp, [esp+4]
    pop  ebp

    ; Return
    ret

; (4) restoreStackPointer, (8)methodPointer
GLOBAL _C(_slvm_dynrun_returnToMethod_trampoline)
_C(_slvm_dynrun_returnToMethod_trampoline):
    ; Store the method pointer in EAX
    mov  eax, [esp+8]

    ; Restore the stack frame.
    mov  esp, [esp+4]
    pop  ebp

    ; Jump to the method
    jmp eax

GLOBAL _C(_slvm_dynrun_switch_to_smalltalk)

; Stack segment header
; (-8) header;
; (-12) stackPointer;
; (-16) framePointer;
; (-20) linkPointer;
; (-24) threadData;
; (-28) segmentSize;
; (-32) reserved;

; (8) stackSegment (12) entryPoint
_C(_slvm_dynrun_switch_to_smalltalk):
; (SLVM_ExecutionStackSegmentHeader *stackSegment, SLVM_ThreadStackData *threadData, void *entryPoint);
    push   ebp
    mov  ebp,esp

    push   ebx
    push   edi
    push   esi

    mov  ebx, [ebp+8]
    mov  eax, [ebp+12]

    ; Fetch the thread data pointer
    mov  ecx, [ebx-24]

    ; Store the old C pointers
    push   dword [ecx+8]
    push   dword [ecx+4]
    push   dword [ecx+0]

    ; Store the C pointers
    mov  [ecx+0],esp
    mov  [ecx+4],ebp
    mov  dword [ecx+8], .LbackToCReturnPointer

    ; Switch to the smalltalk stack.
    mov  esp, [ebx-12]
    mov  ebp, [ebx-16]

    ; Jump to the function
    jmp eax

.LbackToCReturnPointer:
    ; TODO: Is this part, actually used?
    ; Restore the old C pointers
    pop  dword [ecx+0]
    pop  dword [ecx+4]
    pop  dword [ecx+8]

    ; Restore the saved registers and return
    pop  esi
    pop  edi
    pop  ebx

    mov  esp,ebp
    pop  ebp
    ret

GLOBAL _C(_slvm_returnToC_trampoline)
_C(_slvm_returnToC_trampoline):

    ; Fetch the stack segment pointer
    mov  ebx,esp
    and  ebx,-4096
    add  ebx,4096

    ; Fetch the thread data
    mov  ecx, [ebx-24]

    ; Store the smalltalk stack pointers.
    mov  [ebx-12],esp
    mov  [ebx-16],ebp

    ; Restore the C stack
    mov  esp, [ecx+0]
    mov  ebp, [ecx+4]

    ; Return to C
    push   dword [ecx+8]
    ret

; Primitives trampolines
GLOBAL _C(slvm_dynrun_primitive_dispatch)
GLOBAL _C(slvm_dynrun_primitive_dispatch_trampoline)
_C(slvm_dynrun_primitive_dispatch_trampoline):
    push  ebp
    mov  ebp,esp

    ; Store the stack pointer
    mov  edx,esp

    ; Fetch the stack segment pointer
    mov  ebx,esp
    and  ebx,-4096
    add  ebx,4096

    ; Fetch the thread data
    mov  ecx, [ebx-24]

    ; Store the smalltalk stack pointers.
    mov  [ebx-12],esp
    mov  [ebx-16],ebp

    ; Restore the C stack
    mov  esp, [ecx+0]
    mov  ebp, [ecx+4]

    ; Pass the smalltalk stack pointer
    push  edx
    call eax
    hlt
