#ifndef SLVM_CLASSES_COLLECTIONS_H
#define SLVM_CLASSES_COLLECTIONS_H

#include "kernel.h"
#include "magnitude.h"

typedef SLVM_Object SLVM_Collection;
    typedef struct SLVM_HashedCollection_ SLVM_HashedCollection;
        typedef struct SLVM_HashedCollection_ SLVM_Dictionary;
            typedef struct SLVM_HashedCollection_ SLVM_IdentityDictionary;
            typedef struct SLVM_MethodDictionary_ SLVM_MethodDictionary;
    typedef SLVM_Collection SLVM_SequenceableCollection;
        typedef struct SLVM_OrderedCollection_ SLVM_OrderedCollection;
        typedef SLVM_SequenceableCollection SLVM_ArrayedCollection;
            typedef struct SLVM_Array_ SLVM_Array;

struct SLVM_HashedCollection_
{
    SLVM_Collection _base_;
    SLVM_SmallInteger tally;
    SLVM_Array* array;
};

struct SLVM_MethodDictionary_
{
    SLVM_Dictionary _base_;
    SLVM_Oop keys[];
};

struct SLVM_OrderedCollection_
{
    SLVM_SequenceableCollection _base_;
    SLVM_Array *array;
    SLVM_SmallInteger firstIndex;
    SLVM_SmallInteger lastIndex;
};

struct SLVM_Array_
{
    SLVM_ArrayedCollection _base_;
    SLVM_Oop data[];
};

/**
 * Collection based hierarchy.
 */
SLVM_DECLARE_KERNEL_CLASS(Collection);
    SLVM_DECLARE_KERNEL_CLASS(HashedCollection);
        SLVM_DECLARE_KERNEL_CLASS(Dictionary);
            SLVM_DECLARE_KERNEL_CLASS(IdentityDictionary);
            SLVM_DECLARE_KERNEL_CLASS(MethodDictionary);
    SLVM_DECLARE_KERNEL_CLASS(SequenceableCollection);
        SLVM_DECLARE_KERNEL_CLASS(ArrayedCollection);
            SLVM_DECLARE_KERNEL_CLASS(Array);
        SLVM_DECLARE_KERNEL_CLASS(OrderedCollection);

#endif /* SLVM_CLASSES_COLLECTIONS_H */
