#include "slvm/classes/context.h"

SLVM_IMPLEMENT_KERNEL_CLASS(CompiledMethod, Object)
SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(PrimitiveMethod, Object, OF_INDEXABLE_8, 0)

SLVM_IMPLEMENT_KERNEL_CLASS(Message, Object)
SLVM_IMPLEMENT_KERNEL_CLASS(MethodContext, Object)
SLVM_IMPLEMENT_KERNEL_CLASS(BlockClosure, Object)

SLVM_IMPLEMENT_KERNEL_CLASS(Mutex, Object)
SLVM_IMPLEMENT_KERNEL_CLASS(Semaphore, Object)

SLVM_PrimitiveMethod *slvm_PrimitiveMethod_make(SLVM_PrimitiveFunction function)
{
    SLVM_PrimitiveMethod *result = SLVM_KNEW(PrimitiveMethod, sizeof(void*));
    result->entryPoint = function;
    return result;
}
