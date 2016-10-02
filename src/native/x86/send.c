#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "slvm/dynrun.h"

typedef struct
{
    void *framePointer;
    void *returnPointer;
    uint16_t oopArgumentCount;
    uint16_t nativeArgumentSize;

    SLVM_Oop selector;
    SLVM_Oop receiver;
    SLVM_Oop oopArguments[];
} MessageSendRequiredArguments;

typedef struct
{
    void *framePointer;
    void *returnPointer;
} MessageSendResultStackFrame;

extern void _slvm_dynrun_returnToSender_trap(void *restoreStackPointer, void* returnValue);
extern void _slvm_dynrun_returnToMethod_trap(void *restoreStackPointer, void* methodPointer);
extern SLVM_Oop _slvm_dynrun_smalltalk_sendWithArguments(void *entryPoint, uint32_t argumentDescription, SLVM_Oop receiver, SLVM_Oop *oopArguments, void *nativeArguments);

/* Since the message send calling convention is different to C calling convention, we cannot just return. */
void slvm_dynrun_returnToSender(void *stackPointer, void* returnValue)
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
    resultStackFrame->framePointer = requiredArguments->framePointer;
    resultStackFrame->returnPointer = requiredArguments->returnPointer;

    /* Set the return value and restore the stack. */
    _slvm_dynrun_returnToSender_trap(resultStackFrame, returnValue);
}

void* slvm_dynrun_send_dispatch(int senderCallingConvention, void *stackPointer)
{
    SLVM_Oop method;
    SLVM_Oop result;
    SLVM_ObjectHeader *methodHeader;
    SLVM_Behavior *behavior;
    SLVM_CompiledMethod *compiledMethod;
    SLVM_PrimitiveMethod *primitiveMethod;
    unsigned int targetCallingConvention;
    unsigned int methodClass;

    MessageSendRequiredArguments *requiredArguments = (MessageSendRequiredArguments*)stackPointer;
    /*
    printf("Send dispatch %p %d %d: ", stackPointer, requiredArguments->oopArgumentCount, requiredArguments->nativeArgumentSize);
    slvm_String_printLine((SLVM_String*)requiredArguments->selector);
    */

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
            targetCallingConvention = slvm_CompiledMethod_getCallingConvention(compiledMethod);
            /* Invoke the compiled method. Using the correct trampoline for
               adjusting for the calling conventions. */
            if(senderCallingConvention == SLVM_CC_Smalltalk)
            {
                printf("Send from Smalltalk stack\n");
                if(targetCallingConvention == SLVM_CC_Smalltalk)
                {
                    /* Adjust the stack pointer and jump to the called method */
                    _slvm_dynrun_returnToMethod_trap(stackPointer, (void*)(compiledMethod->entryPoint - 1));
                    printf("_slvm_dynrun_returnToMethod_trap should not have returned.\n");
                    abort();
                }
                else
                {
                    printf("TODO: Send from Smalltalk stack to C stack\n");
                }
            }
            else if(senderCallingConvention == SLVM_CC_CDecl)
            {
                if(targetCallingConvention == SLVM_CC_CDecl)
                {
                    /* Adjust the stack pointer and jump to the called method */
                    _slvm_dynrun_returnToMethod_trap(stackPointer, (void*)(compiledMethod->entryPoint - 1));
                    printf("_slvm_dynrun_returnToMethod_trap should not have returned.\n");
                    abort();
                }
                else if(targetCallingConvention == SLVM_CC_Smalltalk)
                {
                    /* Push again the arguments in the stack. */
                    result = _slvm_dynrun_smalltalk_sendWithArguments(
                        (void*) (compiledMethod->entryPoint - 1),
                        requiredArguments->oopArgumentCount | (requiredArguments->nativeArgumentSize << 16),
                        requiredArguments->receiver, requiredArguments->oopArguments,
                        &requiredArguments->oopArguments[requiredArguments->oopArgumentCount]);
                    return (void*) result;
                }
            }
            else
            {
                fprintf(stderr, "Unsupported sender calling convention: %d\n", senderCallingConvention);
                abort();
            }
        }
        else if(methodClass == SLVM_KCI_PrimitiveMethod)
        {
            /* Create the primitive context */
            SLVM_PrimitiveContext context = {
                .stackPointer = stackPointer,
                .selector = requiredArguments->selector, .receiver = requiredArguments->receiver,
                .oopArgumentCount = requiredArguments->oopArgumentCount, .oopArguments = requiredArguments->oopArguments,
                .nativeArgumentSize = requiredArguments->nativeArgumentSize, .nativeArguments = &requiredArguments->oopArguments[requiredArguments->oopArgumentCount]
            };

            /* Call the primitive. */
            primitiveMethod = (SLVM_PrimitiveMethod*)method;
            result = primitiveMethod->entryPoint(&context);

            /* Return adjusting for the caller convention. */
            if(senderCallingConvention == SLVM_CC_Smalltalk)
            {
                slvm_dynrun_returnToSender(stackPointer, (void*)result);
                printf("Primitive return to smalltalk return should not be reached\n");
                abort();
            }
            else
            {
                assert(senderCallingConvention == SLVM_CC_CDecl);
                return (void*)result;
            }
        }
        else
        {
            printf("Found strange method: %p %d\n", (void*)method, methodClass);
        }
    }

    /* Should never reach here. */
    abort();
}
