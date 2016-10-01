#ifndef SLVM_CLASSES_CONTEXT_H
#define SLVM_CLASSES_CONTEXT_H

#include "kernel.h"

typedef struct SLVM_CompiledMethod_ SLVM_CompiledMethod;
typedef struct SLVM_PrimitiveMethod_ SLVM_PrimitiveMethod;

typedef SLVM_Object SLVM_Message;
typedef SLVM_Object SLVM_MethodContext;
typedef SLVM_Object SLVM_BlockClosure;

typedef SLVM_Object SLVM_Mutex;
typedef SLVM_Object SLVM_Semaphore;

typedef SLVM_Oop (*SLVM_PrimitiveFunction) (SLVM_Oop selector, SLVM_Oop receiver,
    size_t oopArgumentCount, SLVM_Oop *oopArguments,
    size_t nativeArgumentSize, void *nativeArguments);

struct SLVM_CompiledMethod_
{
    SLVM_Object _header_;
    SLVM_Oop entryPoint;
    SLVM_Oop literals[];
};

struct SLVM_PrimitiveMethod_
{
    SLVM_Object _header_;
    SLVM_PrimitiveFunction entryPoint;
};

SLVM_DECLARE_KERNEL_CLASS(CompiledMethod)
SLVM_DECLARE_KERNEL_CLASS(PrimitiveMethod)

SLVM_DECLARE_KERNEL_CLASS(Message)
SLVM_DECLARE_KERNEL_CLASS(MethodContext)
SLVM_DECLARE_KERNEL_CLASS(BlockClosure)

SLVM_DECLARE_KERNEL_CLASS(Mutex)
SLVM_DECLARE_KERNEL_CLASS(Semaphore)

SLVM_PrimitiveMethod *slvm_PrimitiveMethod_make(SLVM_PrimitiveFunction function);

#endif /* SLVM_CLASSES_CONTEXT_H */
