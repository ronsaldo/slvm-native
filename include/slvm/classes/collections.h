#ifndef SLVM_CLASSES_COLLECTIONS_H
#define SLVM_CLASSES_COLLECTIONS_H

#include "kernel.h"
#include "magnitude.h"

typedef SLVM_Object SLVM_Collection;
    typedef struct SLVM_HashedCollection_ SLVM_HashedCollection;
        typedef SLVM_HashedCollection SLVM_Dictionary;
            typedef SLVM_Dictionary SLVM_IdentityDictionary;
                typedef struct SLVM_SystemDictionary_ SLVM_SystemDictionary;
            typedef struct SLVM_MethodDictionary_ SLVM_MethodDictionary;
        typedef SLVM_HashedCollection SLVM_Set;
            typedef SLVM_Set SLVM_IdentitySet;
            typedef struct SLVM_WeakSet_ SLVM_WeakSet;
    typedef SLVM_Collection SLVM_SequenceableCollection;
        typedef struct SLVM_OrderedCollection_ SLVM_OrderedCollection;
        typedef SLVM_SequenceableCollection SLVM_ArrayedCollection;
            typedef struct SLVM_Array_ SLVM_Array;
                typedef SLVM_Array SLVM_WeakArray;
            typedef struct SLVM_ByteArray_ SLVM_ByteArray;
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

struct SLVM_SystemDictionary_
{
    SLVM_OrderedCollection *cachedClassNames;
    SLVM_OrderedCollection *cachedNonClassNames;
};

struct SLVM_WeakSet_
{
    SLVM_HashedCollection _base_;
    SLVM_Oop flag;
};

struct SLVM_MethodDictionary_
{
    SLVM_Dictionary _base_;
    SLVM_Array *keys;
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

struct SLVM_ByteArray_
{
    SLVM_ArrayedCollection _base_;
    uint8_t data[];
};

struct SLVM_ByteString_
{
    SLVM_ArrayedCollection _base_;
    uint8_t data[];
};

struct SLVM_WideString_
{
    SLVM_ArrayedCollection _base_;
    uint32_t data[];
};

struct SLVM_ByteSymbol_
{
    SLVM_ArrayedCollection _base_;
    uint8_t data[];
};

struct SLVM_WideSymbol_
{
    SLVM_ArrayedCollection _base_;
    uint32_t data[];
};

/**
 * Collection based hierarchy.
 */
SLVM_DECLARE_KERNEL_CLASS(Collection);
    SLVM_DECLARE_KERNEL_CLASS(HashedCollection);
        SLVM_DECLARE_KERNEL_CLASS(Set);
            SLVM_DECLARE_KERNEL_CLASS(IdentitySet);
            SLVM_DECLARE_KERNEL_CLASS(WeakSet);
        SLVM_DECLARE_KERNEL_CLASS(Dictionary);
            SLVM_DECLARE_KERNEL_CLASS(IdentityDictionary);
                SLVM_DECLARE_KERNEL_CLASS(SystemDictionary);
            SLVM_DECLARE_KERNEL_CLASS(MethodDictionary);
    SLVM_DECLARE_KERNEL_CLASS(SequenceableCollection);
        SLVM_DECLARE_KERNEL_CLASS(ArrayedCollection);
            SLVM_DECLARE_KERNEL_CLASS(Array);
                SLVM_DECLARE_KERNEL_CLASS(WeakArray);
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

/**
 * Arrays
 */
SLVM_Array *slvm_Array_new(size_t n);
SLVM_WeakArray *slvm_WeakArray_new(size_t n);
SLVM_ByteArray *slvm_ByteArray_new(size_t n);

/**
 * Sets
 */
typedef uint32_t (*SLVM_HashFunction) (SLVM_Oop oop);
typedef int (*SLVM_EqualsFunction) (SLVM_Oop first, SLVM_Oop second);

SLVM_WeakSet *slvm_WeakSet_new(size_t n);

intptr_t slvm_WeakSet_scanFor(SLVM_WeakSet *set, SLVM_Oop object, SLVM_HashFunction hashFunction, SLVM_EqualsFunction equalsFunction);
SLVM_Oop slvm_WeakSet_find(SLVM_WeakSet *set, SLVM_Oop object, SLVM_HashFunction hashFunction, SLVM_EqualsFunction equalsFunction);
void slvm_WeakSet_add(SLVM_WeakSet *set, SLVM_Oop object, SLVM_HashFunction hashFunction, SLVM_EqualsFunction equalsFunction);

/**
 * Dictionary
 */
SLVM_Dictionary *slvm_Dictionary_new(SLVM_Class *clazz);
SLVM_Dictionary *slvm_Dictionary_newWithCapacity(SLVM_Class *clazz, size_t n);

SLVM_MethodDictionary *slvm_MethodDictionary_new();
SLVM_MethodDictionary *slvm_MethodDictionary_newWithCapacity(size_t n);
void slvm_MethodDictionary_atPut(SLVM_MethodDictionary *dictionary, SLVM_Oop key, SLVM_Oop value);
SLVM_Oop slvm_MethodDictionary_atOrNil(SLVM_MethodDictionary *dictionary, SLVM_Oop key);

SLVM_Association *slvm_IdentityDictionary_associationAt(SLVM_IdentityDictionary *dictionary, SLVM_Oop key);
void slvm_IdentityDictionary_addAssociation(SLVM_IdentityDictionary *dictionary, SLVM_Association *association);

void slvm_IdentityDictionary_atPutWithAssociationClass(SLVM_IdentityDictionary *dictionary, SLVM_Oop key, SLVM_Oop value, SLVM_Class *associationClass);
void slvm_IdentityDictionary_atPut(SLVM_IdentityDictionary *dictionary, SLVM_Oop key, SLVM_Oop value);
SLVM_Oop slvm_IdentityDictionary_atOrNil(SLVM_IdentityDictionary *dictionary, SLVM_Oop key);

SLVM_SystemDictionary *slvm_SystemDictionary_new();
void slvm_SystemDictionary_atPut(SLVM_SystemDictionary *dictionary, SLVM_Oop key, SLVM_Oop value);

/**
 * Symbols and string
 */
uint32_t slvm_ByteArrayData_stringHash(const uint8_t *data, size_t size, uint32_t initialHash);
uint32_t slvm_CString_hash(const char *cstring);

uint32_t slvm_String_hash(SLVM_String *object);
uint32_t slvm_ByteString_hash(SLVM_ByteString *object, uint32_t initialHash);
uint32_t slvm_WideString_hash(SLVM_WideString *object, uint32_t initialHash);

int slvm_String_equals(SLVM_String *first, SLVM_String *second);
int slvm_ByteString_equals(SLVM_ByteString *first, SLVM_ByteString *second);
int slvm_WideString_equals(SLVM_WideString *first, SLVM_WideString *second);

SLVM_ByteString *slvm_String_convertCString(const char *string);

SLVM_Symbol *slvm_Symbol_internString(SLVM_String *string);
SLVM_Symbol *slvm_Symbol_internCString(const char *cstring);

void slvm_String_print(SLVM_String *object);
void slvm_String_printLine(SLVM_String *object);
void slvm_ByteString_print(SLVM_ByteString *object);
void slvm_WideString_print(SLVM_WideString *object);

#endif /* SLVM_CLASSES_COLLECTIONS_H */
