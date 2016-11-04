#include "slvm/classes/context.h"
#include "slvm/classes/collections.h"

SLVM_IMPLEMENT_KERNEL_CLASS(CompiledMethod, Object)
SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(PrimitiveMethod, Object, OF_INDEXABLE_8, 0)

SLVM_IMPLEMENT_KERNEL_CLASS(Message, Object)
SLVM_IMPLEMENT_KERNEL_CLASS(MethodContext, Object)
SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(BlockClosure, Object, OF_VARIABLE_SIZE_IVARS, 3)
    SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(FullBlockClosure, BlockClosure, OF_VARIABLE_SIZE_IVARS, 4)

SLVM_IMPLEMENT_KERNEL_CLASS(Mutex, Object)
SLVM_IMPLEMENT_KERNEL_CLASS(Semaphore, Object)

SLVM_PrimitiveMethod *slvm_PrimitiveMethod_make(SLVM_PrimitiveFunction function)
{
    SLVM_PrimitiveMethod *result = SLVM_KNEW(PrimitiveMethod, sizeof(void*));
    result->entryPoint = function;
    return result;
}

/* Block closure */
static SLVM_Oop slvm_BlockClosure_primitive_activateClosure(SLVM_PrimitiveContext *context)
{
    SLVM_BlockClosure *blockClosure;
    SLVM_CompiledMethod *method;

    blockClosure = (SLVM_BlockClosure *)context->receiver;
    if(slvm_decodeSmallInteger(blockClosure->numArgs) != slvm_ArgumentDescriptor_make(context->oopArgumentCount, context->nativeArgumentSize))
        slvm_primitiveFailWithError(context, SLVM_PrimitiveError_InvalidArgumentCount);

    if(slvm_getClassIndexFromOop(blockClosure->startpc) != SLVM_KCI_CompiledMethod)
        slvm_primitiveFailWithError(context, SLVM_PrimitiveError_InvalidArgument);

    /* Replace the activation context with the block closure*/
    method = (SLVM_CompiledMethod*)blockClosure->startpc;
    return slvm_primitiveReplaceContextWithCompiledMethodActivation(context, method);
}

extern void slvm_internal_init_context(void)
{
    /* BlockClosure */

    /* Closure activation messages. Up to 15 arguments. */
    SLVM_KCLASS_ADD_PRIMITIVE(BlockClosure, "value", activateClosure);
    SLVM_KCLASS_ADD_PRIMITIVE(BlockClosure, "value:", activateClosure);
    SLVM_KCLASS_ADD_PRIMITIVE(BlockClosure, "value:value:", activateClosure);
    SLVM_KCLASS_ADD_PRIMITIVE(BlockClosure, "value:value:value:", activateClosure);
    SLVM_KCLASS_ADD_PRIMITIVE(BlockClosure, "value:value:value:value:", activateClosure);
    SLVM_KCLASS_ADD_PRIMITIVE(BlockClosure, "value:value:value:value:value:", activateClosure);
    SLVM_KCLASS_ADD_PRIMITIVE(BlockClosure, "value:value:value:value:value:value:", activateClosure);
    SLVM_KCLASS_ADD_PRIMITIVE(BlockClosure, "value:value:value:value:value:value:value:", activateClosure);
    SLVM_KCLASS_ADD_PRIMITIVE(BlockClosure, "value:value:value:value:value:value:value:value:", activateClosure);
    SLVM_KCLASS_ADD_PRIMITIVE(BlockClosure, "value:value:value:value:value:value:value:value:value:", activateClosure);
    SLVM_KCLASS_ADD_PRIMITIVE(BlockClosure, "value:value:value:value:value:value:value:value:value:value:", activateClosure);
    SLVM_KCLASS_ADD_PRIMITIVE(BlockClosure, "value:value:value:value:value:value:value:value:value:value:value:", activateClosure);
    SLVM_KCLASS_ADD_PRIMITIVE(BlockClosure, "value:value:value:value:value:value:value:value:value:value:value:value:", activateClosure);
    SLVM_KCLASS_ADD_PRIMITIVE(BlockClosure, "value:value:value:value:value:value:value:value:value:value:value:value:value:", activateClosure);
    SLVM_KCLASS_ADD_PRIMITIVE(BlockClosure, "value:value:value:value:value:value:value:value:value:value:value:value:value:value:", activateClosure);
    SLVM_KCLASS_ADD_PRIMITIVE(BlockClosure, "value:value:value:value:value:value:value:value:value:value:value:value:value:value:value:", activateClosure);
}
