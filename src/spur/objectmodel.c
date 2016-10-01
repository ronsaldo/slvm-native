#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "slvm/objectmodel.h"
#include "slvm/classes.h"

#ifdef SLVM_SPUR_OBJECT_MODEL
static unsigned int slvm_classTableBaseSize = 0;
static unsigned int slvm_classTableSize = 0;

/**
 * The global class table
 */
SLVM_Behavior** slvm_classTable[4096];

/* The first page is special. It contains only "known" class indices. */
static SLVM_Behavior* slvm_classTableFirstPage[1024];

void slvm_spur_initialize(void)
{
    /* Initialize the first page with nil*/
    int i;
    for(i = 0; i < 1024; ++i)
        slvm_classTableFirstPage[i] = (SLVM_Behavior*)&slvm_nil;

    /* Add the compact class indices to the table. */
    #define COMPACT_CLASS_INDEX(className, index) \
        slvm_classTableFirstPage[index] = (SLVM_Behavior*)&slvm_kernel_class_ ## className; \
        slvm_classTableFirstPage[256 + index] = (SLVM_Behavior*)&slvm_kernel_metaclass_ ## className;
    #define COMPACT_CLASS_ALIAS_INDEX(className, index) COMPACT_CLASS_INDEX(className, index)

#include "slvm/classes/compactIndices.inc"

    #undef COMPACT_CLASS_INDEX
    #undef COMPACT_CLASS_ALIAS_INDEX

    /* Set the initial size of the table. */
    slvm_classTableBaseSize = 512;
    slvm_classTableSize = 0;
    slvm_classTable[0] = slvm_classTableFirstPage;
}

void slvm_spur_shutdown(void)
{
}

static SLVM_ObjectHeader *slvm_spur_instantiate_object(unsigned int format, size_t slotCount)
{
    size_t totalSize;
    size_t preheaderSize;
    uint8_t *rawResult;
    uint64_t *rawResult64;
    SLVM_ObjectHeader *result;

    /* Compute the preheader size. */
    if(slotCount >= 255)
        preheaderSize = 8 + 16; // Big pre-header with alignment
    else
        preheaderSize = 8; // Small pre-header.

    /* Compute the total object size. */
    totalSize = preheaderSize + sizeof(SLVM_ObjectHeader) + slotCount * sizeof(SLVM_Oop);

    /* Allocate the object itself. */
    rawResult = (uint8_t*)malloc(totalSize);
    rawResult64 = (uint64_t*)rawResult;

    /* Set the object pre-header */
    if(slotCount >= 255)
    {
        rawResult64[0] = 1; /* Big object flag. */
        rawResult64[2] = slotCount; /* Slot count*/
    }
    else
    {
        rawResult64[0] = 0;
    }

    /* Initialize the object header. */
    result = (SLVM_ObjectHeader*)(rawResult + preheaderSize);
    memset(result, 0, sizeof(SLVM_ObjectHeader));
    result->slotCount = slotCount >= 255 ? 255 : slotCount;
    result->identityHash = ((size_t)result) >> 4;
    return result;
}

static unsigned int slvm_spur_format_multiplier(unsigned int format)
{
    switch(format)
    {
    case OF_INDEXABLE_64:
        return 1;

    case OF_INDEXABLE_32:
    case OF_INDEXABLE_32_1:
        return sizeof(SLVM_Oop) / 4;

    case OF_INDEXABLE_16:
    case OF_INDEXABLE_16_1:
    case OF_INDEXABLE_16_2:
    case OF_INDEXABLE_16_3:
        return sizeof(SLVM_Oop) / 2;

    case OF_INDEXABLE_8:
    case OF_INDEXABLE_8_1:
    case OF_INDEXABLE_8_2:
    case OF_INDEXABLE_8_3:
    case OF_INDEXABLE_8_4:
    case OF_INDEXABLE_8_5:
    case OF_INDEXABLE_8_6:
    case OF_INDEXABLE_8_7:
        return sizeof(SLVM_Oop);

    case OF_COMPILED_METHOD:
    case OF_COMPILED_METHOD_1:
    case OF_COMPILED_METHOD_2:
    case OF_COMPILED_METHOD_3:
    case OF_COMPILED_METHOD_4:
    case OF_COMPILED_METHOD_5:
    case OF_COMPILED_METHOD_6:
    case OF_COMPILED_METHOD_7:
        return sizeof(SLVM_Oop);

    default:
        return 1;
    }
}

static unsigned int slvm_spur_formatExtraSize(unsigned int format)
{
    switch(format)
    {
    case OF_INDEXABLE_32_1: return 1;

    case OF_INDEXABLE_16_1: return 1;
    case OF_INDEXABLE_16_2: return 2;
    case OF_INDEXABLE_16_3: return 3;

    case OF_INDEXABLE_8_1: return 1;
    case OF_INDEXABLE_8_2: return 2;
    case OF_INDEXABLE_8_3: return 3;
    case OF_INDEXABLE_8_4: return 4;
    case OF_INDEXABLE_8_5: return 5;
    case OF_INDEXABLE_8_6: return 6;
    case OF_INDEXABLE_8_7: return 7;

    case OF_COMPILED_METHOD_1: return 1;
    case OF_COMPILED_METHOD_2: return 2;
    case OF_COMPILED_METHOD_3: return 3;
    case OF_COMPILED_METHOD_4: return 4;
    case OF_COMPILED_METHOD_5: return 5;
    case OF_COMPILED_METHOD_6: return 6;
    case OF_COMPILED_METHOD_7: return 7;

    default:
        return 0;
    }
}

size_t slvm_basicSize(SLVM_Oop object)
{
    SLVM_ObjectHeader *header;
    unsigned int format;
    if(slvm_oopIsImmediate(object))
        return 0;

    header = (SLVM_ObjectHeader*)object;
    format = header->objectFormat;
    if(header->slotCount == 255)
        return ((uint64_t*)header)[-1]*slvm_spur_format_multiplier(format) - slvm_spur_formatExtraSize(format);
    else
        return header->slotCount*slvm_spur_format_multiplier(format) - slvm_spur_formatExtraSize(format);
}

/**
 * Behavior methods
 */
SLVM_ProtoObject *slvm_Behavior_basicNew(SLVM_Behavior *behavior, size_t variableSize)
{
    SLVM_ProtoObject *result;
    unsigned int format;
    size_t fixedSlotCount;
    size_t slotCount;
    size_t extraSlotCount;
    size_t elementSize;
    size_t remainingSize;
    uint8_t *objectData;
    SLVM_Oop *objectOopData;
    size_t i;
    assert(!slvm_isNil(behavior));

    /* Decode the class format. */
    format = slvm_Behavior_decodeFormat(behavior->format);
    fixedSlotCount = slvm_Behavior_decodeFixedSize(behavior->format);

    /* Compute the actual number of slots. */
    slotCount = fixedSlotCount;
    switch(format)
    {
    case OF_INDEXABLE_64:
        if(sizeof(SLVM_Oop) == 4)
            extraSlotCount = variableSize*2;
        else
            extraSlotCount = variableSize;
        elementSize = 8;
        break;
    case OF_INDEXABLE_32:
        elementSize = 4;
        if(sizeof(SLVM_Oop) == 4)
            extraSlotCount = variableSize;
        else
            extraSlotCount = (variableSize*4 + 7) / 4;
        break;
	case OF_INDEXABLE_16:
        elementSize = 2;
        extraSlotCount = (variableSize*2 + sizeof(SLVM_Oop) - 1) / sizeof(SLVM_Oop);
        break;
	case OF_INDEXABLE_8:
	case OF_COMPILED_METHOD:
        elementSize = 1;
        extraSlotCount = (variableSize + sizeof(SLVM_Oop) - 1) / sizeof(SLVM_Oop);
        break;
    default:
        extraSlotCount = variableSize;
        elementSize = sizeof(SLVM_Oop);
        break;
    }

    /* Extra padding elements are encoded in the object format. */
    remainingSize = extraSlotCount*sizeof(SLVM_Oop) - variableSize*elementSize;
    slotCount += extraSlotCount;

    /* Instantiate the object. */
    result = (SLVM_ProtoObject*)slvm_spur_instantiate_object(format, slotCount);
    result->_header_.classIndex = behavior->_base_._header_.identityHash;
    result->_header_.objectFormat = format + remainingSize;

    /* Initialize the object content. */
    objectData = ((uint8_t*)result) + sizeof(SLVM_ObjectHeader);
    if(format < OF_INDEXABLE_64 || format >= OF_COMPILED_METHOD)
    {
        objectOopData = (SLVM_Oop*)objectData;
        for(i = 0; i < slotCount; ++i)
            objectOopData[i] = slvm_nilOop;
    }
    else if(format < OF_COMPILED_METHOD)
    {
        memset(objectData, 0, sizeof(SLVM_Oop)*slotCount);
    }

    return result;
}

/**
 * Regstration of array of roots
 */
void slvm_dynrun_registerArrayOfRoots(SLVM_Oop *array, size_t numberOfElements)
{
}

void slvm_dynrun_unregisterArrayOfRoots(SLVM_Oop *array, size_t numberOfElements)
{
}

void slvm_dynrun_registerStaticHeap(void *start, size_t size)
{
}

void slvm_dynrun_unregisterStaticHeap(void *start, size_t size)
{
}

#endif /* SLVM_SPUR_OBJECT_MODEL */
