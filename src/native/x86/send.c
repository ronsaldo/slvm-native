#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "slvm/dynrun.h"

typedef struct
{
    void *framePointer;
    void *returnPointer;
} MessageReturnFrame;

typedef struct
{
    uint16_t oopArgumentCount;
    uint16_t nativeArgumentSize;
    SLVM_Oop selector;
    void *framePointer;
    void *returnPointer;
} MessageSmalltalkMetadata;

typedef struct
{
    void *framePointer;
    void *returnPointer;
    uint16_t oopArgumentCount;
    uint16_t nativeArgumentSize;
    SLVM_Oop selector;
} MessageCMetadata;

typedef struct
{
    union
    {
        MessageSmalltalkMetadata smalltalk;
        MessageCMetadata c;
    };

    SLVM_Oop receiver;
    SLVM_Oop oopArguments[];
} MessageSendRequiredArguments;

typedef struct
{
    void *framePointer;
    void *returnPointer;
    intptr_t arguments[];
} RuntimePrimitiveArguments;

extern void _slvm_dynrun_returnToSender_trampoline(void *restoreStackPointer, void* returnValue);
extern void _slvm_dynrun_returnToMethod_trampoline(void *restoreStackPointer, void* methodPointer);
extern void _slvm_returnToC_trampoline(void);

extern SLVM_Oop _slvm_dynrun_switch_to_smalltalk(void *stackSegment, void *entryPoint);

extern void _slvm_dynrun_pop_stack_segment_trap(void);

static void slvm_dynrun_returnToSmalltalkSender(MessageSendRequiredArguments *arguments, void* returnValue)
{
    uint8_t *nativeArguments = (uint8_t*)&arguments->oopArguments[arguments->smalltalk.oopArgumentCount];
    MessageReturnFrame *returnFrame = (MessageReturnFrame*)(nativeArguments + arguments->smalltalk.nativeArgumentSize - sizeof(MessageReturnFrame));

    returnFrame->returnPointer = arguments->smalltalk.returnPointer;
    returnFrame->framePointer = arguments->smalltalk.framePointer;
    _slvm_dynrun_returnToSender_trampoline(&returnFrame->framePointer, returnValue);
}

#define SLVM_RUNTIME_PRIMITIVE(name) void slvm_dynrun_primitive_ ## name(RuntimePrimitiveArguments *_stackPointer)
#define SLVM_RUNTIME_PRIMITIVE_ARG(type, index) ((type*)(_stackPointer->arguments + (index)))
#define SLVM_RUNTIME_PRIMITIVE_RETURN(popCount, returnValue) { \
    MessageReturnFrame *_returnFrame = (MessageReturnFrame*)(((uint8_t*)&_stackPointer->arguments[((popCount) + 3) & (-4)]) - sizeof(MessageReturnFrame)); \
    _returnFrame->returnPointer = _stackPointer->returnPointer; \
    _returnFrame->framePointer = _stackPointer->framePointer; \
    _slvm_dynrun_returnToSender_trampoline(&_returnFrame->framePointer, (void*)(returnValue)); \
}

static void slvm_dynrun_convertSendMetadataToSmalltalkFromC(MessageCMetadata *source, MessageSmalltalkMetadata *dest)
{
    dest->oopArgumentCount = source->oopArgumentCount;
    dest->nativeArgumentSize = source->nativeArgumentSize;
    dest->selector = source->selector;
    dest->framePointer = source->framePointer;
    dest->returnPointer = source->returnPointer;
}

void slvm_dynrun_new_stack_segment(uint8_t* currentStackHead, uint32_t argumentDescriptor)
{
    uint16_t nativeArgumentSize;
    uint16_t oopArgumentCount;
    size_t totalArgumentSize;
    void *senderReturnPointer;
    void **newSegmentPointers;
    void **oldSegmentPointers;
    SLVM_ExecutionStackSegmentHeader *newSegment;
    SLVM_ExecutionStackSegmentHeader *currentSegment;

    /* Compute the totalargument size that has to be copied. */
    nativeArgumentSize = argumentDescriptor >> 16;
    oopArgumentCount = argumentDescriptor & 0xFFFF;
    totalArgumentSize = nativeArgumentSize + (oopArgumentCount + /*Receiver */ 1 + /* Return pointers */ 2) * sizeof(void*);

    /* Get the current segment. */
    currentSegment = (SLVM_ExecutionStackSegmentHeader*) (currentStackHead - sizeof(SLVM_ExecutionStackSegmentHeader));

    /* Allocate the new segment */
    newSegment = slvm_ExecutionStack_createNewSegment();
    newSegment->stackPointer -= totalArgumentSize;

    /* Copy the arguments and the return pointers. */
    memcpy(newSegment->stackPointer, currentSegment->stackPointer, totalArgumentSize);

    /* Fix the return pointers. */
    newSegment->stackPointer -= sizeof(void*);
    newSegmentPointers = (void**)newSegment->stackPointer;
    senderReturnPointer = newSegmentPointers[2];
    newSegmentPointers[2] = &_slvm_dynrun_pop_stack_segment_trap;
    newSegmentPointers[0] = newSegment->framePointer;

    /* Fix the old segment for returning. */
    currentSegment->stackPointer += totalArgumentSize - sizeof(void*)*2;
    oldSegmentPointers = (void**)currentSegment->stackPointer;
    oldSegmentPointers[0] = currentSegment->framePointer;
    oldSegmentPointers[1] = senderReturnPointer;

    /* Return to the new segment. */
    /*puts("Move to new segment");*/
    _slvm_dynrun_returnToSender_trampoline(newSegment->stackPointer, 0);
}

void slvm_dynrun_pop_stack_segment(uint8_t* currentStackHead, void *returnValue)
{
    SLVM_ExecutionStackSegmentHeader *previousSegment;
    SLVM_ExecutionStackSegmentHeader *currentSegment;

    /* Get the current segment. */
    currentSegment = (SLVM_ExecutionStackSegmentHeader*) (currentStackHead - sizeof(SLVM_ExecutionStackSegmentHeader));

    /* Get the previous segment. */
    assert(currentSegment->header.previous);
    previousSegment = (SLVM_ExecutionStackSegmentHeader*) ((uint8_t*)currentSegment->header.previous + sizeof(SLVM_LinkedListNode) - sizeof(SLVM_ExecutionStackSegmentHeader));

    /* Pop the old segment. */
    slvm_ExecutionStack_popSegment(currentSegment);

    /* Return to the sender. */
    /*puts("Move to older segment");*/
    _slvm_dynrun_returnToSender_trampoline(previousSegment->stackPointer, returnValue);
}

static void* slvm_dynrun_send_activateMethod(int senderCallingConvention, void *stackPointer, SLVM_Oop method)
{
    SLVM_Oop result;
    SLVM_ObjectHeader *methodHeader;
    SLVM_CompiledMethod *compiledMethod;
    SLVM_PrimitiveMethod *primitiveMethod;
    SLVM_ExecutionStackSegmentHeader *stackSegment;
    unsigned int targetCallingConvention;
    unsigned int methodClass;
    unsigned int totalArgumentSize;
    int isStackFrameThreadBase;
    MessageSmalltalkMetadata smalltalkConverted;

    MessageSendRequiredArguments *requiredArguments = (MessageSendRequiredArguments*)stackPointer;
    MessageSmalltalkMetadata *smalltalk = &requiredArguments->smalltalk;

    /* Convert the sending metadata layout to the Smalltalk convention. */
    if(senderCallingConvention == SLVM_CC_CDecl)
    {
        slvm_dynrun_convertSendMetadataToSmalltalkFromC(&requiredArguments->c, &smalltalkConverted);
        smalltalk = &smalltalkConverted;
    }

    if(slvm_isNil(method))
    {
        printf("[%p] TODO: Send does not understand.\nSelector: ", (void*)requiredArguments->receiver);
        slvm_String_printLine((SLVM_String*)smalltalk->selector);
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
                if(targetCallingConvention == SLVM_CC_Smalltalk)
                {
                    /* Adjust the stack pointer and jump to the called method */
                    _slvm_dynrun_returnToMethod_trampoline(&smalltalk->framePointer, (void*)(compiledMethod->entryPoint - 1));
                    printf("_slvm_dynrun_returnToMethod_trap should not have returned.\n");
                    abort();
                }
                else
                {
                    printf("TODO: Send from Smalltalk stack to C stack\n");
                    /* We need to change back to the C stack. */
                }
            }
            else if(senderCallingConvention == SLVM_CC_CDecl)
            {
                if(targetCallingConvention == SLVM_CC_CDecl)
                {
                    /* Adjust the stack pointer and jump to the called method */
                    _slvm_dynrun_returnToMethod_trampoline(&requiredArguments->c.framePointer, (void*)(compiledMethod->entryPoint - 1));
                    printf("_slvm_dynrun_returnToMethod_trap should not have returned.\n");
                    abort();
                }
                else if(targetCallingConvention == SLVM_CC_Smalltalk)
                {
                    /* We need to change the stack */
                    stackSegment = slvm_ExecutionStack_getValidSegment();
                    printf("Stack segment %p\n", stackSegment);

                    isStackFrameThreadBase = stackSegment->stackPointer == (void*)stackSegment->threadData;

                    /* Copy the arguments. */
                    totalArgumentSize = (smalltalk->oopArgumentCount + 1) * sizeof(SLVM_Oop) + smalltalk->nativeArgumentSize;
                    totalArgumentSize = (totalArgumentSize + 15) & (-16); /* 16 Byte Alignment */
                    stackSegment->stackPointer -= totalArgumentSize;
                    memcpy(stackSegment->stackPointer, &requiredArguments->receiver, totalArgumentSize);

                    /* Store the backward trampoline pointer. */
                    stackSegment->stackPointer -= 4;
                    *((void**)stackSegment->stackPointer) = &_slvm_returnToC_trampoline;

                    /* Push again the arguments in the stack. */
                    result = _slvm_dynrun_switch_to_smalltalk(&stackSegment[1], (void*) (compiledMethod->entryPoint - 1));
                    if(isStackFrameThreadBase)
                        slvm_ExecutionStack_popSegment(stackSegment);

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
                .errorCode = SLVM_PrimitiveError_Success,
                .contextCallingConvention = senderCallingConvention,
                .stackPointer = stackPointer,
                .selector = smalltalk->selector, .receiver = requiredArguments->receiver,
                .oopArgumentCount = smalltalk->oopArgumentCount, .oopArguments = requiredArguments->oopArguments,
                .nativeArgumentSize = smalltalk->nativeArgumentSize, .nativeArguments = &requiredArguments->oopArguments[smalltalk->oopArgumentCount]
            };

            /* Call the primitive. */
            primitiveMethod = (SLVM_PrimitiveMethod*)method;
            result = primitiveMethod->entryPoint(&context);

            /* Check the error code. */
            if(context.errorCode != SLVM_PrimitiveError_Success)
            {
                fprintf(stderr, "Primitive without fallback context failed! Aborting.");
                slvm_String_printLine((SLVM_String*)smalltalk->selector);
                abort();
            }

            /* Return adjusting for the caller convention. */
            if(senderCallingConvention == SLVM_CC_Smalltalk)
            {
                slvm_dynrun_returnToSmalltalkSender(requiredArguments, (void*)result);
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

void* slvm_dynrun_send_dispatch(int senderCallingConvention, void *stackPointer)
{
    SLVM_Oop method;
    SLVM_Behavior *behavior;
    MessageSmalltalkMetadata smalltalkConverted;

    MessageSendRequiredArguments *requiredArguments = (MessageSendRequiredArguments*)stackPointer;
    MessageSmalltalkMetadata *smalltalk = &requiredArguments->smalltalk;

    /* Convert the sending metadata layout to the Smalltalk convention. */
    if(senderCallingConvention == SLVM_CC_CDecl)
    {
        slvm_dynrun_convertSendMetadataToSmalltalkFromC(&requiredArguments->c, &smalltalkConverted);
        smalltalk = &smalltalkConverted;
    }

    /*puts("Send entered");
    printf("Send[%p] dispatch to %p: %p %d %d: ", (void*)smalltalk.selector, (void*)requiredArguments->receiver, stackPointer, smalltalk.oopArgumentCount, smalltalk.nativeArgumentSize);
    slvm_String_printLine((SLVM_String*)smalltalk.selector);
    if(smalltalk.oopArgumentCount)
        printf("Arg 0: %p\n", (void*)requiredArguments->oopArguments[0]);*/

    /* Get the receiver class. */
    behavior = slvm_getClassFromOop(requiredArguments->receiver);
    if(slvm_isNil(behavior))
    {
        fprintf(stderr, "Invalid object class.");
        abort();
    }

    /* Look up the method. */
    method = slvm_Behavior_lookup(behavior, smalltalk->selector);
    return slvm_dynrun_send_activateMethod(senderCallingConvention, stackPointer, method);
}

extern SLVM_Oop slvm_primitiveReplaceContextWithCompiledMethodActivation(SLVM_PrimitiveContext *context, SLVM_CompiledMethod *method)
{
    return (SLVM_Oop)slvm_dynrun_send_activateMethod(context->contextCallingConvention, context->stackPointer, (SLVM_Oop)method);
}

/* Marry this context*/
SLVM_RUNTIME_PRIMITIVE(marryThisContext)
{
    printf("TODO: Marry thisContext in frame: %p\n", *SLVM_RUNTIME_PRIMITIVE_ARG(void*, 0));
    SLVM_RUNTIME_PRIMITIVE_RETURN(1, slvm_nilOop);
}

/* Full block closure creation. */
SLVM_RUNTIME_PRIMITIVE(createFullBlockClosure)
{
    SLVM_Oop outerContext = *SLVM_RUNTIME_PRIMITIVE_ARG(SLVM_Oop, 0);
    SLVM_Oop receiver = *SLVM_RUNTIME_PRIMITIVE_ARG(SLVM_Oop, 1);
    SLVM_CompiledMethod *compiledMethod = *SLVM_RUNTIME_PRIMITIVE_ARG(SLVM_CompiledMethod*, 2);
    uint32_t copiedParametersSize = *SLVM_RUNTIME_PRIMITIVE_ARG(uint32_t, 3);

    SLVM_FullBlockClosure *closure = SLVM_KNEW(FullBlockClosure, copiedParametersSize);
    closure->_base_.outerContext = outerContext;
    closure->_base_.numArgs = compiledMethod->argumentDescriptor;
    closure->_base_.startpc = (SLVM_Oop)compiledMethod;
    closure->receiver = receiver;
    printf("Created closure %p\n", closure);

    SLVM_RUNTIME_PRIMITIVE_RETURN(4, closure);
}

/* Temporal vector create. */
SLVM_RUNTIME_PRIMITIVE(tempVectorCreate)
{
    SLVM_Array *result = slvm_Array_new(*SLVM_RUNTIME_PRIMITIVE_ARG(size_t, 0));
    SLVM_RUNTIME_PRIMITIVE_RETURN(1, result);
}
