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
            typedef SLVM_ArrayedCollection SLVM_String;
                typedef struct SLVM_ByteString_ SLVM_ByteString;
                typedef struct SLVM_WideString_ SLVM_WideString;
                typedef SLVM_String SLVM_Symbol;
                    typedef struct SLVM_ByteSymbol_ SLVM_ByteSymbol;
                    typedef struct SLVM_WideSymbol_ SLVM_WideSymbol;

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
            SLVM_DECLARE_KERNEL_CLASS(Bitmap);
            SLVM_DECLARE_KERNEL_CLASS(ByteArray);
            SLVM_DECLARE_KERNEL_CLASS(FloatArray);
            SLVM_DECLARE_KERNEL_CLASS(IntegerArray);
            SLVM_DECLARE_KERNEL_CLASS(WordArray);
            SLVM_DECLARE_KERNEL_CLASS(String);
                SLVM_DECLARE_KERNEL_CLASS(ByteString);
                SLVM_DECLARE_KERNEL_CLASS(WideString);
                SLVM_DECLARE_KERNEL_CLASS(Symbol);
                    SLVM_DECLARE_KERNEL_CLASS(ByteSymbol);
                    SLVM_DECLARE_KERNEL_CLASS(WideSymbol);
        SLVM_DECLARE_KERNEL_CLASS(OrderedCollection);

#endif /* SLVM_CLASSES_COLLECTIONS_H */
