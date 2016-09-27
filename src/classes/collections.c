#include "slvm/classes/collections.h"

SLVM_IMPLEMENT_KERNEL_CLASS(Collection, Object);
    SLVM_IMPLEMENT_KERNEL_CLASS(HashedCollection, Collection);
        SLVM_IMPLEMENT_KERNEL_CLASS(Dictionary, HashedCollection);
            SLVM_IMPLEMENT_KERNEL_CLASS(IdentityDictionary, Dictionary);
            SLVM_IMPLEMENT_KERNEL_CLASS(MethodDictionary, Dictionary);
    SLVM_IMPLEMENT_KERNEL_CLASS(SequenceableCollection, Collection);
        SLVM_IMPLEMENT_KERNEL_CLASS(ArrayedCollection, SequenceableCollection);
            SLVM_IMPLEMENT_KERNEL_CLASS(Array, ArrayedCollection);
            SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(Bitmap, ArrayedCollection, OF_INDEXABLE_32, 0);
            SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(ByteArray, ArrayedCollection, OF_INDEXABLE_8, 0);
            SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(FloatArray, ArrayedCollection, OF_INDEXABLE_32, 0);
            SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(IntegerArray, ArrayedCollection, OF_INDEXABLE_32, 0);
            SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(WordArray, ArrayedCollection, OF_INDEXABLE_32, 0);
            SLVM_IMPLEMENT_KERNEL_CLASS(String, ArrayedCollection);
                SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(ByteString, String, OF_INDEXABLE_8, 0);
                SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(WideString, String, OF_INDEXABLE_32, 0);
                SLVM_IMPLEMENT_KERNEL_CLASS(Symbol, String);
                    SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(ByteSymbol, Symbol, OF_INDEXABLE_8, 0);
                    SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(WideSymbol, Symbol, OF_INDEXABLE_32, 0);
        SLVM_IMPLEMENT_KERNEL_CLASS(OrderedCollection, SequenceableCollection);
