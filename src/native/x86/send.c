#include <stdio.h>
#include <stdlib.h>
#include "slvm/dynrun.h"

typedef struct
{
    void *returnPointer;
    uint16_t oopArgumentCount;
    uint16_t nativeArgumentSize;

    SLVM_Oop selector;
    SLVM_Oop receiver;
    SLVM_Oop oopArguments[];
} MessageSendRequiredArguments;

typedef struct
{
    void *returnPointer;
} MessageSendResultStackFrame;

extern void _slvm_dynrun_returnToSend_trap(void *restoreStackPointer, void* returnValue);

/* Since the message send calling convention is different to C calling convention, we cannot just return. */
void slvm_dynrun_returnToSend(void *stackPointer, void* returnValue)
{
    size_t totalArgumentsSize;
    MessageSendRequiredArguments *requiredArguments;
    MessageSendResultStackFrame *resultStackFrame;

    /* Compute the value that has to be cleaned. */
    requiredArguments = (MessageSendRequiredArguments*)stackPointer;
    totalArgumentsSize = sizeof(MessageSendRequiredArguments)
        + requiredArguments->nativeArgumentSize
        + requiredArguments->oopArgumentCount*sizeof(SLVM_Oop);

    /* Move the return pointer up in the stack. */
    resultStackFrame = (MessageSendResultStackFrame*) ((uint8_t*)stackPointer + totalArgumentsSize - sizeof(MessageSendResultStackFrame));
    resultStackFrame->returnPointer = requiredArguments->returnPointer;

    /* Set the return value and restore the stack. */
    _slvm_dynrun_returnToSend_trap(resultStackFrame, returnValue);
}

void slvm_dynrun_send(void *stackPointer)
{
    SLVM_Oop method;
    SLVM_Oop result;
    SLVM_ObjectHeader *methodHeader;
    SLVM_Behavior *behavior;
    SLVM_CompiledMethod *compiledMethod;
    SLVM_PrimitiveMethod *primitiveMethod;
    unsigned int methodClass;

    MessageSendRequiredArguments *requiredArguments = (MessageSendRequiredArguments*)stackPointer;

    /* Get the receiver class. */
    behavior = slvm_getClassFromOop(requiredArguments->receiver);
    if(slvm_isNil(behavior))
    {
        fprintf(stderr, "Invalid object class.");
        abort();
    }

    /* Look up the method. */
    method = slvm_Behavior_lookup(behavior, requiredArguments->selector);
    if(slvm_isNil(method))
    {
        printf("TODO: Send does not understand.\nSelector: ");
        slvm_String_printLine((SLVM_String*)requiredArguments->selector);
    }
    else
    {
        methodHeader = (SLVM_ObjectHeader*)method;
        methodClass = methodHeader->classIndex;

        if(methodClass == SLVM_KCI_CompiledMethod)
        {
            compiledMethod = (SLVM_CompiledMethod*)method;
            printf("Found a compiled method: %p\n", compiledMethod);
        }
        else if(methodClass == SLVM_KCI_PrimitiveMethod)
        {
            /* Create the primitive context */
            PrimitiveContext context = {
                .stackPointer = stackPointer,
                .selector = requiredArguments->selector, .receiver = requiredArguments->receiver,
                .oopArgumentCount = requiredArguments->oopArgumentCount, .oopArguments = requiredArguments->oopArguments,
                .nativeArgumentSize = requiredArguments->nativeArgumentSize, .nativeArguments = &requiredArguments->oopArguments[requiredArguments->oopArgumentCount]
            };

            /* Call the primitive. */
            primitiveMethod = (SLVM_PrimitiveMethod*)method;
            result = primitiveMethod->entryPoint(&context);

            /* Return through the trampoline. */
            return slvm_dynrun_returnToSend(stackPointer, (void*)result);
        }
        else
        {
            printf("Found strange method: %p %d\n", (void*)method, methodClass);
        }
    }

    /* Should never reach here. */
    abort();
}
