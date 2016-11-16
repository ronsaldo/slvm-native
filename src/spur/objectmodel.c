#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "slvm/objectmodel.h"
#include "slvm/memory.h"
#include "slvm/classes.h"
#include "slvm/lifecycle.h"

#ifdef SLVM_SPUR_OBJECT_MODEL
#define FORWARDING_POINTER_TAG_MASK 3

static unsigned int slvm_classTablePageCount = 0;
static unsigned int slvm_classTableBaseSize = 0;
static unsigned int slvm_classTableSize = 0;

static SLVM_LinkedList staticHeaps;
static SLVM_StackHeapInformation compactingHeap;

typedef void (*SLVM_SpurHeapIterator) (SLVM_HeapInformation *heapInformation, SLVM_Oop *forwardingPointer, SLVM_ObjectHeader *objectHeader, size_t oopSlotCount, size_t totalSlotCount);

static void slvm_spur_heap_iterate(SLVM_HeapInformation *heapInformation, SLVM_SpurHeapIterator iterator)
{
    uint64_t oopSlotCount;
    uint64_t totalSlotCount;
    uint64_t *forwardingPointer;
    SLVM_ObjectHeader *objectHeader;
    uint8_t *position;
    uint8_t *start = (uint8_t*)heapInformation->start;
    uint8_t *end = start + heapInformation->size;

    position = start;
    while(position < end)
    {
        /* Get a pointer to the forwarding pointer. */
        forwardingPointer = (uint64_t*)position;

        /* We are using the least significant bit to signal for size preheader. */
        if(*forwardingPointer & 1)
        {
            position += 24;
            objectHeader = (SLVM_ObjectHeader *)position;
            if(objectHeader->objectFormat >= OF_MIXED_OBJECT)
            {
                oopSlotCount = forwardingPointer[2] >> 32;
                totalSlotCount = forwardingPointer[2] & 0xFFFFFFFF;
            }
            else
            {
                oopSlotCount = totalSlotCount = forwardingPointer[2];
            }
            assert(objectHeader->slotCount == 255);
        }
        else
        {
            position += 8;
            objectHeader = (SLVM_ObjectHeader *)position;
            oopSlotCount = totalSlotCount = objectHeader->slotCount;
            assert(objectHeader->slotCount < 255);
        }

        /* Call the iterator. */
        iterator(heapInformation, (SLVM_Oop *)forwardingPointer, objectHeader, (size_t)oopSlotCount, (size_t)totalSlotCount);

        /* Increase the position. */
        position += sizeof(SLVM_ObjectHeader) + totalSlotCount * sizeof(SLVM_Oop);

        /* Align the position into a 16 byte boundary. */
        position = (uint8_t*) (((uintptr_t)position + 15) & (-16));
    }

    assert(position == end);
}

SLVM_Oop *slvm_spur_heap_getForwardingPointerForObject(SLVM_Oop oop)
{
    SLVM_ObjectHeader *header = (SLVM_ObjectHeader *)oop;
    if(header->slotCount == 255)
        return (SLVM_Oop*) ((uint8_t*)header - 24);
    else
        return (SLVM_Oop*) ((uint8_t*)header - 8);
}

/**
 * The global class table
 */
SLVM_Behavior** slvm_classTable[4096];

/* The first page is special. It contains only "known" class indices. */
static SLVM_Behavior* slvm_classTableFirstPage[1024];

static void slvm_spur_create_compactingHeap(void)
{
    /* Create the compacting heap. */
    int result = slvm_StackHeap_create(&compactingHeap, SLVM_SPUR_DEFAULT_COMPACTING_HEAP_CAPACITY, SMPF_Readable | SMPF_Writeable);
    if(result != 0)
    {
        fprintf(stderr, "Failed to create the compacting heap\n");
        abort();
    }

    /* Mark the compacting heap as initialized. */
    compactingHeap.base.flags = SHF_Initialized;

    /* Register the compacting heap. */
    slvm_list_addNode(&staticHeaps, (SLVM_LinkedListNode*)&compactingHeap);
}

static void slvm_spur_create_heaps(void)
{
    slvm_spur_create_compactingHeap();
}

void slvm_spur_initialize(void)
{
    /* Create the spur heaps. */
    slvm_spur_create_heaps();

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
    slvm_classTablePageCount = 1;
    slvm_classTable[0] = slvm_classTableFirstPage;
}

void slvm_objectmodel_registerBehavior(SLVM_Behavior *behavior)
{
    int i;
    SLVM_Behavior** page;
    unsigned int classIndex = slvm_classTableBaseSize + slvm_classTableSize;
    unsigned int pageIndex = classIndex >> 12;
    unsigned int elementIndex = classIndex & 1023;

    if(pageIndex >= slvm_classTablePageCount)
    {
        page = slvm_classTable[pageIndex] = (SLVM_Behavior**)malloc(sizeof(slvm_classTableFirstPage));
        for(i = 0; i < 1024; ++i)
            page[i] = (SLVM_Behavior*)&slvm_nil;

        slvm_classTablePageCount++;
    }

    behavior->_base_._header_.identityHash = classIndex;
    slvm_classTable[pageIndex][elementIndex] = behavior;
    slvm_classTableSize++;
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
        preheaderSize = 8 + 16; // Big pre-header for alignment
    else
        preheaderSize = 8; // Small pre-header.

    /* Compute the total object size. */
    totalSize = preheaderSize + sizeof(SLVM_ObjectHeader) + slotCount * sizeof(SLVM_Oop);

    /* Allocate the object itself. */
    rawResult = (uint8_t*)slvm_StackHeap_allocate(&compactingHeap, 16, totalSize);
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

static SLVM_ObjectHeader *slvm_spur_instantiate_mixed_object(unsigned int format, size_t oopSlotCount, size_t slotCount)
{
    const size_t preheaderSize = 8 + 16;
    size_t totalSize;
    uint8_t *rawResult;
    uint64_t *rawResult64;
    SLVM_ObjectHeader *result;

    /* Compute the total object size. */
    totalSize = preheaderSize + sizeof(SLVM_ObjectHeader) + slotCount * sizeof(SLVM_Oop);

    /* Allocate the object itself. */
    rawResult = (uint8_t*)slvm_StackHeap_allocate(&compactingHeap, 16, totalSize);
    rawResult64 = (uint64_t*)rawResult;

    /* Set the object pre-header */
    rawResult64[0] = 1; /* Big object flag. */
    rawResult64[2] = slotCount | (((uint64_t)oopSlotCount) << 32ul); /* Slot count*/

    /* Initialize the object header. */
    result = (SLVM_ObjectHeader*)(rawResult + preheaderSize);
    memset(result, 0, sizeof(SLVM_ObjectHeader));
    result->slotCount = 255; /* Always big object header here. */
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

    case OF_MIXED_OBJECT:
    case OF_MIXED_OBJECT_1:
    case OF_MIXED_OBJECT_2:
    case OF_MIXED_OBJECT_3:
    case OF_MIXED_OBJECT_4:
    case OF_MIXED_OBJECT_5:
    case OF_MIXED_OBJECT_6:
    case OF_MIXED_OBJECT_7:
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

    case OF_MIXED_OBJECT_1: return 1;
    case OF_MIXED_OBJECT_2: return 2;
    case OF_MIXED_OBJECT_3: return 3;
    case OF_MIXED_OBJECT_4: return 4;
    case OF_MIXED_OBJECT_5: return 5;
    case OF_MIXED_OBJECT_6: return 6;
    case OF_MIXED_OBJECT_7: return 7;

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
    {
        if(format >= OF_MIXED_OBJECT)
            return (((uint64_t*)header)[-1] & 0xFFFFFFFF)*sizeof(SLVM_Oop) - slvm_spur_formatExtraSize(format);
        return ((uint64_t*)header)[-1]*slvm_spur_format_multiplier(format) - slvm_spur_formatExtraSize(format);
    }
    else
        return header->slotCount*slvm_spur_format_multiplier(format) - slvm_spur_formatExtraSize(format);
}

uint8_t *slvm_firstNativeDataPointer(SLVM_Oop value)
{
    SLVM_ObjectHeader *header;
    uint32_t *objectPreHeader;

    header = (SLVM_ObjectHeader*)value;
    assert(slvm_oopIsPointers(value));
    assert(header->objectFormat >= OF_MIXED_OBJECT);

    objectPreHeader = (uint32_t*)value;
    return ((uint8_t*)value) + sizeof(SLVM_ObjectHeader) + objectPreHeader[-1]*sizeof(SLVM_Oop);
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
    assert(format < OF_MIXED_OBJECT);

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
    if(format < OF_INDEXABLE_64)
    {
        objectOopData = (SLVM_Oop*)objectData;
        for(i = 0; i < slotCount; ++i)
            objectOopData[i] = slvm_nilOop;
    }
    else if(format < OF_MIXED_OBJECT)
    {
        memset(objectData, 0, sizeof(SLVM_Oop)*slotCount);
    }

    return result;
}

SLVM_ProtoObject *slvm_Behavior_basicNewMixedNative(SLVM_Behavior *behavior, size_t variableSize, size_t nativeVariableSize)
{
    SLVM_ProtoObject *result;
    unsigned int format;
    size_t fixedSlotCount;
    size_t slotCount;
    size_t extraSlotCount;
    size_t variableContentSize;
    size_t remainingSize;
    size_t oopSlotCount;
    uint8_t *objectData;
    SLVM_Oop *objectOopData;
    size_t i;
    assert(!slvm_isNil(behavior));

    /* Decode the class format. */
    format = slvm_Behavior_decodeFormat(behavior->format);
    fixedSlotCount = slvm_Behavior_decodeFixedSize(behavior->format);

    /* Compute the actual number of slots. */
    slotCount = fixedSlotCount;
    oopSlotCount = slotCount + variableSize;
    variableContentSize = variableSize*sizeof(SLVM_Oop) + nativeVariableSize;
    extraSlotCount = (variableContentSize + sizeof(SLVM_Oop) - 1) / sizeof(SLVM_Oop);

    /* Extra padding elements are encoded in the object format. */
    remainingSize = extraSlotCount*sizeof(SLVM_Oop) - variableContentSize;
    slotCount += extraSlotCount;

    /* Instantiate the object. */
    result = (SLVM_ProtoObject*)slvm_spur_instantiate_mixed_object(format, oopSlotCount, slotCount);
    result->_header_.classIndex = behavior->_base_._header_.identityHash;
    result->_header_.objectFormat = format + remainingSize;

    /* Initialize the object content. */
    objectData = ((uint8_t*)result) + sizeof(SLVM_ObjectHeader);
    objectOopData = (SLVM_Oop*)objectData;
    for(i = 0; i < oopSlotCount; ++i)
        objectOopData[i] = slvm_nilOop;
    memset(&objectOopData[oopSlotCount], 0, sizeof(SLVM_Oop)*(slotCount - oopSlotCount));

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

static void slvm_internal_fixStaticHeap_symbols(SLVM_HeapInformation *heapInformation, SLVM_Oop *forwardingPointer, SLVM_ObjectHeader *header, size_t oopSlotCount, size_t totalSlotCount)
{
    SLVM_Oop becomeTarget;
    /* Clear the forwarding pointer. */
    *forwardingPointer &= ~FORWARDING_POINTER_TAG_MASK;

    /* Forward symbols into an interned version of them.*/
    becomeTarget = (SLVM_Oop)header;
    if(header->classIndex == SLVM_KCI_ByteSymbol || header->classIndex == SLVM_KCI_WideSymbol)
        becomeTarget = (SLVM_Oop)slvm_Symbol_internString((SLVM_String*)header);

    /* Update the forwarding pointer. */
    if(becomeTarget != (SLVM_Oop)header)
        *forwardingPointer |= becomeTarget;
}

static void slvm_internal_fixStaticHeap_globalVariables(SLVM_HeapInformation *heapInformation, SLVM_Oop *forwardingPointer, SLVM_ObjectHeader *header, size_t oopSlotCount, size_t totalSlotCount)
{
    SLVM_Oop becomeTarget;

    /* Clear the forwarding pointer. */
    *forwardingPointer &= ~FORWARDING_POINTER_TAG_MASK;

    /* Forward symbols into an interned version of them.*/
    becomeTarget = (SLVM_Oop)header;
    if(header->classIndex == SLVM_KCI_GlobalVariable)
        becomeTarget = slvm_globals_addIfNotExistent((SLVM_Oop)header);

    /* Update the forwarding pointer. */
    if(becomeTarget != (SLVM_Oop)header)
        *forwardingPointer |= becomeTarget;
}

static void slvm_internal_fixStaticHeap_classVariables(SLVM_HeapInformation *heapInformation, SLVM_Oop *forwardingPointer, SLVM_ObjectHeader *header, size_t oopSlotCount, size_t totalSlotCount)
{
    SLVM_Oop becomeTarget;

    /* Clear the forwarding pointer. */
    *forwardingPointer &= ~FORWARDING_POINTER_TAG_MASK;

    /* Forward symbols into an interned version of them.*/
    becomeTarget = (SLVM_Oop)header;
    if(header->classIndex == SLVM_KCI_ClassVariable)
        becomeTarget = slvm_globals_fixClassVariable((SLVM_Oop)header);

    /* Update the forwarding pointer. */
    if(becomeTarget != (SLVM_Oop)header)
        *forwardingPointer |= becomeTarget;
}

static void slvm_spur_heap_applyNotNullForwarding(SLVM_HeapInformation *heapInformation, SLVM_Oop *forwardingPointer, SLVM_ObjectHeader *header, size_t oopSlotCount, size_t totalSlotCount)
{
    size_t i;
    SLVM_Oop *objectData;
    SLVM_Oop element;
    SLVM_Oop elementForwarded;
    SLVM_Oop *elementForwardingPointer;
    SLVM_Oop heapStart = (SLVM_Oop)heapInformation->start;
    SLVM_Oop heapEnd = heapStart + heapInformation->size;

    /* If the object does not contain pointers, ignore it. */
    if(header->objectFormat >= OF_INDEXABLE_64 && header->objectFormat < OF_MIXED_OBJECT)
        return;

    /* Cast the object. */
    objectData = (SLVM_Oop*)&header[1];
    for(i = 0; i < oopSlotCount; ++i)
    {
        element = objectData[i];
        if(!slvm_oopIsPointers(element))
            continue;

        /* Only process the objects in the same heap. */
        if(element < heapStart || heapEnd <= element)
            continue;

        /* Get the forwarded element. */
        elementForwardingPointer = slvm_spur_heap_getForwardingPointerForObject(element);
        elementForwarded = *elementForwardingPointer & (~FORWARDING_POINTER_TAG_MASK);

        /* Only apply the forwarding if the forwarded element is not null. */
        if(elementForwarded != 0)
            objectData[i] = elementForwarded;
    }
}

static void slvm_internal_fixStaticHeap(SLVM_HeapInformation *heapInformation)
{
    // Fix the symbols first.
    slvm_spur_heap_iterate(heapInformation, &slvm_internal_fixStaticHeap_symbols);
    slvm_spur_heap_iterate(heapInformation, &slvm_spur_heap_applyNotNullForwarding);

    // Fix the global variables.
    slvm_spur_heap_iterate(heapInformation, &slvm_internal_fixStaticHeap_globalVariables);
    slvm_spur_heap_iterate(heapInformation, &slvm_spur_heap_applyNotNullForwarding);
}


static void slvm_internal_fixStaticHeapPass2(SLVM_HeapInformation *heapInformation)
{
    // Fix the class variables.
    slvm_spur_heap_iterate(heapInformation, &slvm_internal_fixStaticHeap_classVariables);
    slvm_spur_heap_iterate(heapInformation, &slvm_spur_heap_applyNotNullForwarding);
}

static void slvm_internal_init_staticHeap(SLVM_HeapInformation *heapInformation)
{
    SLVM_HeapWithPackageInformation *packageInformation;

    if(heapInformation->flags & SHF_Initialized)
        return;

    if(heapInformation->flags & SHF_MayNeedFixingUp)
        slvm_internal_fixStaticHeap(heapInformation);

    if(heapInformation->flags & SHF_HasPackageRegistration)
    {
        packageInformation = (SLVM_HeapWithPackageInformation *)heapInformation;
        packageInformation->packageRegistration();
    }

    if(heapInformation->flags & SHF_MayNeedFixingUp)
        slvm_internal_fixStaticHeapPass2(heapInformation);

    heapInformation->flags |= SHF_Initialized;
}

void slvm_dynrun_registerStaticHeap(SLVM_HeapInformation *heapInformation)
{
    slvm_list_addNode(&staticHeaps, (SLVM_LinkedListNode*)heapInformation);
    if(slvm_dynrun_isKernelInitialized())
        slvm_internal_init_staticHeap(heapInformation);
}

void slvm_dynrun_unregisterStaticHeap(SLVM_HeapInformation *heapInformation)
{
    slvm_list_removeNode(&staticHeaps, (SLVM_LinkedListNode*)heapInformation);
}

void slvm_internal_init_staticHeaps(void)
{
    SLVM_LinkedListNode *currentNode;

    assert(slvm_dynrun_isKernelInitialized());

    for(currentNode = staticHeaps.first; currentNode; currentNode = currentNode->next)
        slvm_internal_init_staticHeap((SLVM_HeapInformation*)currentNode);
}

extern SLVM_Oop slvm_makeInteger(SLVM_Oop value)
{
    return slvm_encodeSmallInteger(value);
}
#endif /* SLVM_SPUR_OBJECT_MODEL */
