#include "slvm/classes/magnitude.h"

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
