#ifndef SLVM_CLASSES_CONTEXT_H
#define SLVM_CLASSES_CONTEXT_H

#include "kernel.h"

typedef struct SLVM_CompiledMethod_ SLVM_CompiledMethod;
typedef struct SLVM_PrimitiveMethod_ SLVM_PrimitiveMethod;

typedef SLVM_Object SLVM_Message;
typedef SLVM_Object SLVM_MethodContext;
typedef struct SLVM_BlockClosure_ SLVM_BlockClosure;
    typedef struct SLVM_FullBlockClosure_ SLVM_FullBlockClosure;

typedef SLVM_Object SLVM_Mutex;
typedef SLVM_Object SLVM_Semaphore;

enum SLVM_CallingConvention
{
    SLVM_CC_Smalltalk = 0,
    SLVM_CC_CDecl,
    SLVM_CC_StdCall,
};

enum SLVM_PrimitiveErrorCode
{
    SLVM_PrimitiveError_Success = 0,
    SLVM_PrimitiveError_Error,
    SLVM_PrimitiveError_InvalidArgumentCount,
    SLVM_PrimitiveError_InvalidArgument,

    SLVM_PrimitiveError_Generic = 1000
};

typedef struct SLVM_PrimitiveContext_
{
    int errorCode;
    int contextCallingConvention;
    void *stackPointer;
    SLVM_Oop selector;
    SLVM_Oop receiver;

    size_t oopArgumentCount;
    SLVM_Oop *oopArguments;

    size_t nativeArgumentSize;
    void *nativeArguments;
} SLVM_PrimitiveContext;

typedef SLVM_Oop (*SLVM_PrimitiveFunction) (SLVM_PrimitiveContext *context);

struct SLVM_CompiledMethod_
{
    SLVM_Object _header_;
    SLVM_Oop entryPoint;
    SLVM_Oop flags;
    SLVM_Oop argumentDescriptor;
    SLVM_Oop annotations;
    SLVM_Oop literals[];
};

struct SLVM_PrimitiveMethod_
{
    SLVM_Object _header_;
    SLVM_PrimitiveFunction entryPoint;
};

struct SLVM_BlockClosure_
{
    SLVM_Object _header_;
    SLVM_Oop outerContext;
    SLVM_Oop startpc;
    SLVM_Oop numArgs;
};

struct SLVM_FullBlockClosure_
{
    SLVM_BlockClosure _base_;
    SLVM_Oop receiver;
};

#define slvm_ArgumentDescriptor_make(oopArgumentCount, nativeArgumentSize) (((nativeArgumentSize) << 16) | (oopArgumentCount))

#define slvm_CompiledMethod_getCallingConvention(method) \
    (slvm_decodeSmallInteger(method->flags) & 7)

SLVM_DECLARE_KERNEL_CLASS(CompiledMethod)
SLVM_DECLARE_KERNEL_CLASS(PrimitiveMethod)

SLVM_DECLARE_KERNEL_CLASS(Message)
SLVM_DECLARE_KERNEL_CLASS(MethodContext)
SLVM_DECLARE_KERNEL_CLASS(BlockClosure)
    SLVM_DECLARE_KERNEL_CLASS(FullBlockClosure)

SLVM_DECLARE_KERNEL_CLASS(Mutex)
SLVM_DECLARE_KERNEL_CLASS(Semaphore)

SLVM_PrimitiveMethod *slvm_PrimitiveMethod_make(SLVM_PrimitiveFunction function);

#define slvm_primitiveFailWithError(context, errorCodeValue) { \
    (context)->errorCode = errorCodeValue; \
    return slvm_nilOop; \
} \

#define slvm_primitiveFail(context) slvm_primitiveFailWithError(context, SLVM_PrimitiveError_Error)

extern SLVM_Oop slvm_primitiveReplaceContextWithCompiledMethodActivation(SLVM_PrimitiveContext *context, SLVM_CompiledMethod *method);

#endif /* SLVM_CLASSES_CONTEXT_H */
