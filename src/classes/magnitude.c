#include <stdio.h>
#include "slvm/classes.h"

SLVM_IMPLEMENT_KERNEL_CLASS(Magnitude, Object);
    SLVM_IMPLEMENT_KERNEL_CLASS(Character, Magnitude);
    SLVM_IMPLEMENT_KERNEL_CLASS(Number, Magnitude);
        SLVM_IMPLEMENT_KERNEL_CLASS(Float, Number);
            SLVM_IMPLEMENT_KERNEL_CLASS(BoxedFloat64, Float);
            SLVM_IMPLEMENT_KERNEL_CLASS(SmallFloat, Float);
        SLVM_IMPLEMENT_KERNEL_CLASS(Fraction, Number);
        SLVM_IMPLEMENT_KERNEL_CLASS(Integer, Number);
            SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(LargeInteger, Integer, OF_INDEXABLE_8, 0);
                SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(LargeNegativeInteger, LargeInteger, OF_INDEXABLE_8, 0);
                SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(LargePositiveInteger, LargeInteger, OF_INDEXABLE_8, 0);
            SLVM_IMPLEMENT_KERNEL_CLASS(SmallInteger, Integer);
    SLVM_IMPLEMENT_KERNEL_CLASS(LookupKey, Number);
        SLVM_IMPLEMENT_KERNEL_CLASS(Association, LookupKey);

SLVM_IMPLEMENT_KERNEL_CLASS(Point, Object);

/**
 * SmallInteger primitives
 */
SLVM_Oop slvm_SmallInteger_primitive_asString(SLVM_PrimitiveContext *context)
{
    SLVM_ByteString *result;
    char buffer[128];
    sprintf(buffer, "%lld", (long long)slvm_decodeSmallInteger(context->receiver));

    result = slvm_String_convertCString(buffer);
    return (SLVM_Oop)result;
}

/**
 * Association
 */
SLVM_Association *slvm_Association_new()
{
    return SLVM_KNEW(Association, 0);
}

SLVM_Association *slvm_Association_make(SLVM_Oop key, SLVM_Oop value)
{
    SLVM_Association *result = slvm_Association_new();
    result->_base_.key = key;
    result->value = value;
    return result;
}

void slvm_internal_init_magnitude(void)
{
    SLVM_KCLASS_ADD_PRIMITIVE(SmallInteger, "asString", asString);
}
