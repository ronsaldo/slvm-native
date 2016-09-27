#ifndef SLVM_CLASSES_CONTEXT_H
#define SLVM_CLASSES_CONTEXT_H

#include "kernel.h"

typedef SLVM_Object SLVM_Message;
typedef SLVM_Object SLVM_MethodContext;
typedef SLVM_Object SLVM_BlockClosure;

typedef SLVM_Object SLVM_Mutex;
typedef SLVM_Object SLVM_Semaphore;

SLVM_DECLARE_KERNEL_CLASS(Message)
SLVM_DECLARE_KERNEL_CLASS(MethodContext)
SLVM_DECLARE_KERNEL_CLASS(BlockClosure)

SLVM_DECLARE_KERNEL_CLASS(Mutex)
SLVM_DECLARE_KERNEL_CLASS(Semaphore)

#endif /* SLVM_CLASSES_CONTEXT_H */
