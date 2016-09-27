#ifndef SLVM_CLASSES_MAGNITUDE_H
#define SLVM_CLASSES_MAGNITUDE_H

#include "kernel.h"

/**
 * Magnitude based kernel hierarchy.
 */
typedef SLVM_Object SLVM_Magnitude;
    typedef SLVM_Oop SLVM_Character;
    typedef SLVM_Object SLVM_Number;
        typedef SLVM_Object SLVM_Float;
            typedef SLVM_Object SLVM_BoxedFloat64;
            typedef SLVM_Oop SLVM_SmallFloat64;
    typedef struct SLVM_Fraction_ SLVM_Fraction;
    typedef SLVM_Object SLVM_Integer;
        typedef struct SLVM_LargeInteger_ SLVM_LargeInteger;
            typedef struct SLVM_LargeInteger_ SLVM_LargeNegativeInteger;
            typedef struct SLVM_LargeInteger_ SLVM_LargePositiveInteger;
        typedef SLVM_SOop SLVM_SmallInteger;
    typedef struct SLVM_LookupKey_ SLVM_LookupKey;
        typedef struct SLVM_Association_ SLVM_Association;

/**
 * Fraction layout.
 */
struct SLVM_Fraction_
{
    SLVM_Number _base_;
    SLVM_Oop numerator;
    SLVM_Oop denominator;
};

/**
 * Large integer layout.
 */
struct SLVM_LargeInteger_
{
    SLVM_Integer _base_;
    uint8_t data[];
};

/**
 * Lookup key layout
 */
struct SLVM_LookupKey_
{
    SLVM_Magnitude _base_;
    SLVM_Oop key;
};

/**
 * Strong association
 */
struct SLVM_Association_
{
    SLVM_LookupKey _base_;
    SLVM_Oop value;
};

/**
 * Declaration of the kernel class objects.
 */
SLVM_DECLARE_KERNEL_CLASS(Magnitude);
    SLVM_DECLARE_KERNEL_CLASS(Character);
    SLVM_DECLARE_KERNEL_CLASS(Number);
        SLVM_DECLARE_KERNEL_CLASS(Float);
            SLVM_DECLARE_KERNEL_CLASS(BoxedFloat64);
            SLVM_DECLARE_KERNEL_CLASS(SmallFloat);
        SLVM_DECLARE_KERNEL_CLASS(Fraction);
        SLVM_DECLARE_KERNEL_CLASS(Integer);
            SLVM_DECLARE_KERNEL_CLASS(LargeInteger);
                SLVM_DECLARE_KERNEL_CLASS(LargeNegativeInteger);
                SLVM_DECLARE_KERNEL_CLASS(LargePositiveInteger);
            SLVM_DECLARE_KERNEL_CLASS(SmallInteger);
    SLVM_DECLARE_KERNEL_CLASS(LookupKey);
        SLVM_DECLARE_KERNEL_CLASS(Association);

#endif /* SLVM_CLASSES_MAGNITUDE_H */
