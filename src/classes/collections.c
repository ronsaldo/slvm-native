#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "slvm/classes.h"
#include "slvm/memory.h"

SLVM_IMPLEMENT_KERNEL_CLASS(Collection, Object);
    SLVM_IMPLEMENT_KERNEL_CLASS(HashedCollection, Collection);
        SLVM_IMPLEMENT_KERNEL_CLASS(Dictionary, HashedCollection);
            SLVM_IMPLEMENT_KERNEL_CLASS(IdentityDictionary, Dictionary);
                SLVM_IMPLEMENT_KERNEL_CLASS(SystemDictionary, IdentityDictionary);
            SLVM_IMPLEMENT_KERNEL_CLASS(MethodDictionary, Dictionary);
        SLVM_IMPLEMENT_KERNEL_CLASS(Set, HashedCollection);
            SLVM_IMPLEMENT_KERNEL_CLASS(IdentitySet, HashedCollection);
            SLVM_IMPLEMENT_KERNEL_CLASS(WeakSet, HashedCollection);
    SLVM_IMPLEMENT_KERNEL_CLASS(SequenceableCollection, Collection);
        SLVM_IMPLEMENT_KERNEL_CLASS(ArrayedCollection, SequenceableCollection);
            SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(Array, ArrayedCollection, OF_VARIABLE_SIZE_NO_IVARS, 0);
                SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(WeakArray, Array, OF_WEAK_VARIABLE_SIZE, 0);
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

/**
 * Kernel collections roots
 */
struct SLVM_KernelCollectionsRoot
{
    /**
     * Symbol class variables
     */
    SLVM_WeakSet *newSymbols;
    SLVM_WeakSet *symbolTable;
};

static struct SLVM_KernelCollectionsRoot collectionRoots;

/**
 * Arrays
 */
SLVM_Array *slvm_Array_new(size_t n)
{
    return (SLVM_Array*)slvm_Behavior_basicNew((SLVM_Behavior*)SLVM_KCLASS(Array), n);
}

SLVM_WeakArray *slvm_WeakArray_new(size_t n)
{
    return (SLVM_WeakArray*)slvm_Behavior_basicNew((SLVM_Behavior*)SLVM_KCLASS(WeakArray), n);
}

SLVM_ByteArray *slvm_ByteArray_new(size_t n)
{
    return (SLVM_ByteArray*)slvm_Behavior_basicNew((SLVM_Behavior*)SLVM_KCLASS(ByteArray), n);
}

/**
 * String
 */
SLVM_ByteString *slvm_String_convertCString(const char *string)
{
    size_t length = strlen(string);
    SLVM_ByteString *object = SLVM_KNEW(ByteString, length);
    memcpy(object->data, string, length);
    return object;
}

int slvm_String_equals(SLVM_String *first, SLVM_String *second)
{
    unsigned int firstClassIndex;
    unsigned int secondClassIndex;
    unsigned int isFirstBytes;
    unsigned int isSecondBytes;
    unsigned int isFirstWide;
    unsigned int isSecondWide;

    /* If we are dealing with the same pointer, they have to be equal. */
    if(first == second)
        return 1;

    firstClassIndex = first->_header_.classIndex;
    secondClassIndex = second->_header_.classIndex;
    isFirstBytes = firstClassIndex == SLVM_KCI_ByteString || secondClassIndex == SLVM_KCI_ByteSymbol;
    isSecondBytes = secondClassIndex == SLVM_KCI_ByteString || secondClassIndex == SLVM_KCI_ByteSymbol;

    /* Dispatch the equality comparison. */
    if(isFirstBytes && isSecondBytes)
        return slvm_ByteString_equals((SLVM_ByteString*)first, (SLVM_ByteString*)second);

    isFirstWide = firstClassIndex == SLVM_KCI_WideString || secondClassIndex == SLVM_KCI_WideSymbol;
    isSecondWide = secondClassIndex == SLVM_KCI_WideString || secondClassIndex == SLVM_KCI_WideSymbol;
    if(isFirstWide && isSecondWide)
        return slvm_WideString_equals((SLVM_WideString*)first, (SLVM_WideString*)second);
    else
        return 0;
}

int slvm_ByteString_equals(SLVM_ByteString *first, SLVM_ByteString *second)
{
    size_t i;
    size_t firstSize = slvm_basicSize((SLVM_Oop)first);
    size_t secondSize = slvm_basicSize((SLVM_Oop)second);
    if(firstSize != secondSize)
        return 0;

    for(i = 0; i < firstSize; ++i)
    {
        if(first->data[i] != second->data[i])
            return 0;
    }

    return 1;
}

int slvm_WideString_equals(SLVM_WideString *first, SLVM_WideString *second)
{
    size_t i;
    size_t firstSize = slvm_basicSize((SLVM_Oop)first);
    size_t secondSize = slvm_basicSize((SLVM_Oop)second);
    if(firstSize != secondSize)
        return 0;

    for(i = 0; i < firstSize; ++i)
    {
        if(first->data[i] != second->data[i])
            return 0;
    }

    return 1;
}

uint32_t slvm_String_hash(SLVM_String *object)
{
    unsigned int classIndex = object->_header_.classIndex;
    if(classIndex == SLVM_KCI_ByteString || classIndex == SLVM_KCI_ByteSymbol)
    {
        return slvm_ByteString_hash((SLVM_ByteString*)object, SLVM_KCI_ByteString);
    }
    else if(classIndex == SLVM_KCI_WideString || classIndex == SLVM_KCI_WideSymbol)
    {
        return slvm_WideString_hash((SLVM_WideString*)object, SLVM_KCI_ByteString);
    }

    fprintf(stderr, "Unsupported string class kind for computing the string hash.");
    abort();
}

uint32_t slvm_ByteArrayData_stringHash(const uint8_t *data, size_t size, uint32_t initialHash)
{
    size_t i;
    uint32_t low;
    uint32_t hash = initialHash & 0xFFFFFFF;

    for(i = 0; i < size; ++i)
    {
        hash += data[i];
        low = hash & 16383;
        hash = (0x260D * low + ((0x260D * (hash >> 14) + 0x0065 * low) & 16383) * 16384) & 0x0FFFFFFF;
    }

    return hash;
}

uint32_t slvm_CString_hash(const char *cstring)
{
    return slvm_ByteArrayData_stringHash((const uint8_t*)cstring, strlen(cstring), SLVM_KCI_ByteString);
}

uint32_t slvm_ByteString_hash(SLVM_ByteString *object, uint32_t initialHash)
{
    size_t size = slvm_basicSize((SLVM_Oop)object);

    return slvm_ByteArrayData_stringHash(object->data, size, initialHash);
}

uint32_t slvm_WideString_hash(SLVM_WideString *object, uint32_t initialHash)
{
    size_t i;
    uint32_t low;
    uint32_t hash = initialHash & 0xFFFFFFF;
    size_t size = slvm_basicSize((SLVM_Oop)object);

    for(i = 0; i < size; ++i)
    {
        hash += object->data[i];
        low = hash & 16383;
        hash = (0x260D * low + ((0x260D * (hash >> 14) + 0x0065 * low) & 16383) * 16384) & 0x0FFFFFFF;
    }

    return hash;
}

void slvm_String_print(SLVM_String *object)
{
    unsigned int classIndex;
    assert(!slvm_isNil(object));

    classIndex = object->_header_.classIndex;
    if(classIndex == SLVM_KCI_ByteString || classIndex == SLVM_KCI_ByteSymbol)
    {
        return slvm_ByteString_print((SLVM_ByteString*)object);
    }
    else if(classIndex == SLVM_KCI_WideString || classIndex == SLVM_KCI_WideSymbol)
    {
        return slvm_WideString_print((SLVM_WideString*)object);
    }
}

void slvm_ByteString_print(SLVM_ByteString *object)
{
    (void)fwrite(object->data, slvm_basicSize((SLVM_Oop)object), 1, stdout);
}

void slvm_WideString_print(SLVM_WideString *object)
{
    /* TODO: implement myself when it is possible. */
}

void slvm_String_printLine(SLVM_String *object)
{
    slvm_String_print(object);
    puts("");
}

static SLVM_Oop slvm_String_primitive_printToStdout(SLVM_PrimitiveContext *context)
{
    slvm_String_printLine((SLVM_String*)context->receiver);
    return context->receiver;
}
/**
 * Symbol
 */
static SLVM_ByteSymbol *slvm_Symbol_convertCString(const char *string)
{
    size_t length = strlen(string);
    SLVM_ByteSymbol *object = SLVM_KNEW(ByteSymbol, length);
    memcpy(object->data, string, length);
    return object;
}

SLVM_Symbol *slvm_Symbol_internString(SLVM_String *string)
{
    SLVM_Oop old;
    SLVM_Symbol *symbol;
    SLVM_ByteSymbol *byteSymbol;
    SLVM_WideSymbol *wideSymbol;
    size_t symbolSize;
    unsigned int classIndex;

    old = slvm_WeakSet_find(collectionRoots.symbolTable, (SLVM_Oop)string, (SLVM_HashFunction)&slvm_String_hash, (SLVM_EqualsFunction)&slvm_String_equals);
    if(!slvm_isNil(old))
        return (SLVM_Symbol*)old;

    old = slvm_WeakSet_find(collectionRoots.newSymbols, (SLVM_Oop)string, (SLVM_HashFunction)&slvm_String_hash, (SLVM_EqualsFunction)&slvm_String_equals);
    if(!slvm_isNil(old))
        return (SLVM_Symbol*)old;

    /* If the string was created by slvm_Symbol_internCString, then string could already be a symbol. */
    classIndex = string->_header_.classIndex;
    if(classIndex == SLVM_KCI_ByteSymbol || classIndex == SLVM_KCI_WideSymbol)
    {
        symbol = (SLVM_Symbol*)string;
    }
    /* String is not a symbol, so convert it into a symbol. */
    else if(classIndex == SLVM_KCI_ByteString)
    {
        symbolSize = slvm_basicSize((SLVM_Oop)string);
        byteSymbol = SLVM_KNEW(ByteSymbol, symbolSize);

        memcpy(byteSymbol->data, ((SLVM_ByteString*)string)->data, symbolSize);
        symbol = (SLVM_Symbol*)byteSymbol;
    }
    else if(classIndex == SLVM_KCI_WideString)
    {
        symbolSize = slvm_basicSize((SLVM_Oop)string);
        wideSymbol = SLVM_KNEW(WideSymbol, symbolSize);

        memcpy(wideSymbol->data, ((SLVM_WideString*)string)->data, symbolSize*4);
        symbol = (SLVM_Symbol*)wideSymbol;
    }
    else
    {
        fprintf(stderr, "Trying to convert a non-string object into a symbol.");
        abort();
    }

    slvm_WeakSet_add(collectionRoots.newSymbols, (SLVM_Oop)symbol, (SLVM_HashFunction)&slvm_String_hash, (SLVM_EqualsFunction)&slvm_String_equals);
    return symbol;
}

SLVM_Symbol *slvm_Symbol_internCString(const char *cstring)
{
    SLVM_ByteSymbol *string = slvm_Symbol_convertCString(cstring);
    return slvm_Symbol_internString((SLVM_String*)string);
}

/**
 * HashedCollection
 */
 static int slvm_HashedCollection_fullCondition(SLVM_HashedCollection *collection)
 {
     size_t arraySize = slvm_basicSize((SLVM_Oop)collection->array);
     return arraySize < 4 || arraySize - slvm_decodeSmallInteger(collection->tally) < arraySize / 4;
 }

/**
 * Dictionary
 */
SLVM_Dictionary *slvm_Dictionary_new(SLVM_Class *clazz)
{
    return slvm_Dictionary_newWithCapacity(clazz, 5);
}

SLVM_Dictionary *slvm_Dictionary_newWithCapacity(SLVM_Class *clazz, size_t n)
{
    SLVM_Dictionary *result = (SLVM_Dictionary*)slvm_Behavior_basicNew((SLVM_Behavior*)clazz, 0);
    result->array = SLVM_KNEW(Array, n);
    result->tally = slvm_encodeSmallInteger(0);
    return result;
}

SLVM_SystemDictionary *slvm_SystemDictionary_new()
{
    SLVM_SystemDictionary *result = (SLVM_SystemDictionary*)slvm_Dictionary_new(SLVM_KCLASS(SystemDictionary));
    return result;
}

/**
 * Identity dictionary
 */
intptr_t slvm_IdentityDictionary_scanFor(SLVM_IdentityDictionary *dictionary, SLVM_Oop key)
{
    SLVM_Association *element;
    size_t arraySize;
    size_t start;
    size_t index;

    arraySize = slvm_basicSize((SLVM_Oop)dictionary->array);
    if(arraySize == 0)
        return -1;

    start = slvm_identityHash(key) % arraySize;
    index = start;
    SLVM_Array *array = dictionary->array;
    do
    {
        element = (SLVM_Association*)array->data[index];
        if(slvm_isNil(element) || element->_base_.key == key)
            return index;

        index = (index + 1) % arraySize;
    } while(index != start);

    return -1;
}

void slvm_IdentityDictionary_grow(SLVM_IdentityDictionary *dictionary)
{
    size_t i;
    size_t arraySize;
    size_t newArraySize;
    SLVM_Array *oldArray;
    SLVM_Association *element;

    /* Duplicate the dictionary capacity. */
    arraySize = slvm_basicSize((SLVM_Oop)dictionary->array);
    newArraySize = arraySize * 2;
    if(newArraySize < 4)
        newArraySize = 4;

    /* Allocate the new array. */
    oldArray = dictionary->array;
    dictionary->tally = slvm_encodeSmallInteger(0);
    dictionary->array = SLVM_KNEW(Array, newArraySize);

    /* Reinsert the elements into the set. */
    for(i = 0; i < arraySize; ++i)
    {
        element = (SLVM_Association*)oldArray->data[i];
        if(!slvm_isNil(element))
            slvm_IdentityDictionary_addAssociation(dictionary, element);
    }
}

SLVM_Association *slvm_IdentityDictionary_associationAt(SLVM_IdentityDictionary *dictionary, SLVM_Oop key)
{
    intptr_t index;

    assert(!slvm_isNil(dictionary));

    /* Scan the element. */
    index  = slvm_IdentityDictionary_scanFor(dictionary, key);
    assert(index >= 0);

    /* Return the value of the element. */
    return (SLVM_Association *)dictionary->array->data[index];
}

void slvm_IdentityDictionary_addAssociation(SLVM_IdentityDictionary *dictionary, SLVM_Association *association)
{
    SLVM_Array *array;
    intptr_t index;

    assert(!slvm_isNil(dictionary));
    assert(!slvm_isNil(association));

    /* Scan the element */
    index = slvm_IdentityDictionary_scanFor(dictionary, association->_base_.key);
    assert(index >= 0);

    /* Put the element at its position. */
    array = dictionary->array;
    if((SLVM_Association*)array->data[index] == association)
    {
        array->data[index] = (SLVM_Oop)association;
    }
    else
    {
        array->data[index] = (SLVM_Oop)association;

        /* Increase the dictionary size. */
        dictionary->tally += slvm_encodeSmallIntegerOffset(1);
        if(slvm_HashedCollection_fullCondition((SLVM_HashedCollection*)dictionary))
            slvm_IdentityDictionary_grow(dictionary);
    }
}

void slvm_IdentityDictionary_atPut(SLVM_IdentityDictionary *dictionary, SLVM_Oop key, SLVM_Oop value)
{
    SLVM_Array *array;
    SLVM_Association *element;
    intptr_t index;

    assert(!slvm_isNil(dictionary));

    /* Scan the element */
    index = slvm_IdentityDictionary_scanFor(dictionary, key);
    assert(index >= 0);

    /* Put the element at its position. */
    array = dictionary->array;
    element = (SLVM_Association*)array->data[index];
    if(slvm_isNil(element))
    {
        array->data[index] = (SLVM_Oop)slvm_Association_make(key, value);

        /* Increase the dictionary size. */
        dictionary->tally += slvm_encodeSmallIntegerOffset(1);
        if(slvm_HashedCollection_fullCondition((SLVM_HashedCollection*)dictionary))
            slvm_IdentityDictionary_grow(dictionary);
    }
    else
    {
        element->value = value;
    }
}

SLVM_Oop slvm_IdentityDictionary_atOrNil(SLVM_IdentityDictionary *dictionary, SLVM_Oop key)
{
    intptr_t index;
    SLVM_Association *element;

    assert(!slvm_isNil(dictionary));

    /* Find the association. */
    index = slvm_IdentityDictionary_scanFor(dictionary, key);
    assert(index >= 0);

    /* Read the value in the association, if not nil. */
    element = (SLVM_Association*)dictionary->array->data[index];
    if(slvm_isNil(element))
        return slvm_nilOop;
    return element->value;
}

/**
 * MethodDictionary
 */
SLVM_MethodDictionary *slvm_MethodDictionary_new()
{
    return slvm_MethodDictionary_newWithCapacity(5);
}

SLVM_MethodDictionary *slvm_MethodDictionary_newWithCapacity(size_t n)
{
    SLVM_MethodDictionary *result = (SLVM_MethodDictionary*)slvm_Dictionary_newWithCapacity(SLVM_KCLASS(MethodDictionary), n);
    result->keys = SLVM_KNEW(Array, n);
    return result;
}

intptr_t slvm_MethodDictionary_scanFor(SLVM_MethodDictionary *dictionary, SLVM_Oop key)
{
    SLVM_Oop element;
    size_t arraySize;
    size_t start;
    size_t index;

    arraySize = slvm_basicSize((SLVM_Oop)dictionary->keys);
    if(arraySize == 0)
        return -1;

    start = slvm_identityHash(key) % arraySize;
    index = start;
    SLVM_Array *array = dictionary->keys;
    do
    {
        element = array->data[index];
        if(slvm_isNil(element) || element == key)
            return index;

        index = (index + 1) % arraySize;
    } while(index != start);

    return -1;
}

void slvm_MethodDictionary_grow(SLVM_MethodDictionary *dictionary)
{
    size_t i;
    size_t arraySize;
    size_t newArraySize;
    SLVM_Array *oldValues;
    SLVM_Array *oldKeys;
    SLVM_Oop element;

    printf("slvm_MethodDictionary_grow\n");

    /* Duplicate the dictionary capacity. */
    arraySize = slvm_basicSize((SLVM_Oop)dictionary->keys);
    newArraySize = arraySize * 2;
    if(newArraySize < 4)
        newArraySize = 4;

    /* Allocate the new array. */
    oldValues = dictionary->_base_.array;
    oldKeys = dictionary->keys;
    dictionary->_base_.tally = slvm_encodeSmallInteger(0);
    dictionary->_base_.array = SLVM_KNEW(Array, newArraySize);
    dictionary->keys = SLVM_KNEW(Array, newArraySize);

    /* Reinsert the elements into the set. */
    for(i = 0; i < arraySize; ++i)
    {
        element = oldKeys->data[i];
        if(!slvm_isNil(element))
            slvm_MethodDictionary_atPut(dictionary, element, oldValues->data[i]);
    }
}

void slvm_MethodDictionary_atPut(SLVM_MethodDictionary *dictionary, SLVM_Oop key, SLVM_Oop value)
{
    intptr_t index;

    assert(!slvm_isNil(dictionary));
    index = slvm_MethodDictionary_scanFor(dictionary, key);
    if(index < 0)
    {
        slvm_MethodDictionary_grow(dictionary);
        return slvm_MethodDictionary_atPut(dictionary, key, value);
    }

    if(slvm_isNil(dictionary->keys->data[index]))
    {
        dictionary->keys->data[index] = key;
        dictionary->_base_.array->data[index] = value;

        /* Increase the dictionary size. */
        dictionary->_base_.tally += slvm_encodeSmallIntegerOffset(1);
        if(slvm_HashedCollection_fullCondition((SLVM_HashedCollection*)dictionary))
            slvm_MethodDictionary_grow(dictionary);
    }
    else
    {
        dictionary->_base_.array->data[index] = value;
    }

}

SLVM_Oop slvm_MethodDictionary_atOrNil(SLVM_MethodDictionary *dictionary, SLVM_Oop key)
{
    intptr_t index;

    assert(!slvm_isNil(dictionary));
    index = slvm_MethodDictionary_scanFor(dictionary, key);
    if(index < 0)
        return slvm_nilOop;

    return dictionary->_base_.array->data[index];
}

/**
 * WeakSet
 */
static SLVM_Array *slvm_WeakSet_newStorage(SLVM_WeakSet *set, size_t storageSize)
{
    size_t i;
    SLVM_Oop flag = set->flag;
    SLVM_Array *result = SLVM_KNEW(Array, storageSize);

    for(i = 0; i < storageSize; ++i)
        result->data[i] = flag;
    return result;
}
SLVM_WeakSet *slvm_WeakSet_new(size_t n)
{
    SLVM_WeakSet *result = SLVM_KNEW(WeakSet, 0);
    result->flag = (SLVM_Oop)SLVM_KNEW(Object, 0);
    result->_base_.tally = slvm_encodeSmallInteger(0);
    result->_base_.array = slvm_WeakSet_newStorage(result, n);
    return result;
}

intptr_t slvm_WeakSet_scanFor(SLVM_WeakSet *set, SLVM_Oop object, SLVM_HashFunction hashFunction, SLVM_EqualsFunction equalsFunction)
{
    SLVM_Oop element;
    SLVM_Oop flag;
    size_t arraySize;
    size_t start;
    size_t index;

    arraySize = slvm_basicSize((SLVM_Oop)set->_base_.array);
    if(arraySize == 0)
        return -1;

    start = hashFunction(object) % arraySize;
    index = start;
    SLVM_Array *array = set->_base_.array;
    flag = set->flag;
    do
    {
        element = array->data[index];
        if(element == flag || equalsFunction(object, element))
            return index;

        index = (index + 1) % arraySize;
    } while(index != start);

    return -1;
}

SLVM_Oop slvm_WeakSet_find(SLVM_WeakSet *set, SLVM_Oop object, SLVM_HashFunction hashFunction, SLVM_EqualsFunction equalsFunction)
{
    SLVM_Oop result;
    intptr_t index = slvm_WeakSet_scanFor(set, object, hashFunction, equalsFunction);
    if(index < 0)
        return slvm_nilOop;

    result = set->_base_.array->data[index];
    if(result == set->flag)
        return slvm_nilOop;
    return result;
}

static void slvm_WeakSet_doAdd(SLVM_WeakSet *set, SLVM_Oop object, SLVM_HashFunction hashFunction, SLVM_EqualsFunction equalsFunction)
{
    intptr_t index = slvm_WeakSet_scanFor(set, object, hashFunction, equalsFunction);
    assert(index >= 0);

    /* Do not increase the size if the element was already in the set. */
    if(set->_base_.array->data[index] == object)
        return;

    set->_base_.array->data[index] = object;
    set->_base_.tally += slvm_encodeSmallIntegerOffset(1);
}

static void slvm_WeakSet_doGrow(SLVM_WeakSet *set, SLVM_HashFunction hashFunction, SLVM_EqualsFunction equalsFunction, size_t newArraySize)
{
    size_t i;
    size_t arraySize;
    SLVM_Array *oldArray;
    SLVM_Oop element;
    SLVM_Oop flag = set->flag;

    /* Allocate the new array. */
    arraySize = slvm_basicSize((SLVM_Oop)set->_base_.array);
    oldArray = set->_base_.array;
    set->_base_.tally = slvm_encodeSmallInteger(0);
    set->_base_.array = slvm_WeakSet_newStorage(set, newArraySize);

    /* Reinsert the elements into the set. */
    for(i = 0; i < arraySize; ++i)
    {
        element = oldArray->data[i];
        if(element != flag)
            slvm_WeakSet_doAdd(set, element, hashFunction, equalsFunction);
    }
}

void slvm_WeakSet_grow(SLVM_WeakSet *set, SLVM_HashFunction hashFunction, SLVM_EqualsFunction equalsFunction)
{
    size_t arraySize;
    size_t newArraySize;
    printf("WeakSet grow\n");

    /* Try to recreate into a set of the same size. This will remove any nil
       element that was added by the GC. */
    arraySize = slvm_basicSize((SLVM_Oop)set->_base_.array);
    slvm_WeakSet_doGrow(set, hashFunction, equalsFunction, arraySize);
    if(!slvm_HashedCollection_fullCondition((SLVM_HashedCollection*)set))
        return;

    /* Try to duplicate the set. */
    newArraySize = arraySize * 2;
    if(newArraySize < 4)
        newArraySize = 4;

    slvm_WeakSet_doGrow(set, hashFunction, equalsFunction, newArraySize);
}

void slvm_WeakSet_add(SLVM_WeakSet *set, SLVM_Oop object, SLVM_HashFunction hashFunction, SLVM_EqualsFunction equalsFunction)
{
    slvm_WeakSet_doAdd(set, object, hashFunction, equalsFunction);

    if(slvm_HashedCollection_fullCondition((SLVM_HashedCollection*)set))
        slvm_WeakSet_grow(set, hashFunction, equalsFunction);
}

/**
 * Collections initialization.
 */
void slvm_internal_init_collections_kernel(void)
{
    /* Create the sets that are used for symbol internation. Symbols are used
     as selectors. */
    collectionRoots.newSymbols = (SLVM_WeakSet*)slvm_WeakSet_new(128);
    collectionRoots.symbolTable = (SLVM_WeakSet*)slvm_WeakSet_new(16);

    slvm_dynrun_registerArrayOfRoots((SLVM_Oop*)&collectionRoots, sizeof(collectionRoots) / sizeof(SLVM_Oop));
}

void slvm_internal_init_collections(void)
{
    /* Symbol class variables. */
    SLVM_KCLASS_VARIABLE_SET(Symbol, NewSymbols, collectionRoots.newSymbols);
    SLVM_KCLASS_VARIABLE_SET(Symbol, SymbolTable, collectionRoots.symbolTable);

    /* Primitives */
    SLVM_KCLASS_ADD_PRIMITIVE(String, "printToStdout", printToStdout);
}
