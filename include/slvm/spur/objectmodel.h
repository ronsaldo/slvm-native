#ifndef SLVM_SPUR_OBJECT_MODEL_H
#define SLVM_SPUR_OBJECT_MODEL_H

#include <stdint.h>
#include <stddef.h>

/**
 * A pointer into an object or an immediate value.
 */
typedef uintptr_t SLVM_Oop;
typedef intptr_t SLVM_SOop;

#if defined(__i386__)
#define SLVM_SPUR_32 1
#elif defined(__x86_64__) || defined(__amd64__)
#define SLVM_SPUR_64 1
#else
#error Unsupported architecture
#endif

#define SLVM_SPUR_OBJECT_MODEL 1

#if SLVM_SPUR_32
#define SLVM_SPUR_DEFAULT_COMPACTING_HEAP_CAPACITY (512 << 20) /* 512 MB */
#else
#define SLVM_SPUR_DEFAULT_COMPACTING_HEAP_CAPACITY (((size_t)10) << ((size_t)30)) /* 10 GB */
#endif

/**
 * Object format
 */
enum ObjectFormat
{
	OF_EMPTY = 0,
	OF_FIXED_SIZE = 1,
	OF_VARIABLE_SIZE_NO_IVARS = 2,
	OF_VARIABLE_SIZE_IVARS = 3,
	OF_WEAK_VARIABLE_SIZE = 4,
	OF_WEAK_FIXED_SIZE = 5,
	OF_INDEXABLE_64 = 9,
	OF_INDEXABLE_32 = 10,
	OF_INDEXABLE_32_1,
	OF_INDEXABLE_16 = 12,
	OF_INDEXABLE_16_1,
	OF_INDEXABLE_16_2,
	OF_INDEXABLE_16_3,
	OF_INDEXABLE_8 = 16,
	OF_INDEXABLE_8_1,
	OF_INDEXABLE_8_2,
	OF_INDEXABLE_8_3,
	OF_INDEXABLE_8_4,
	OF_INDEXABLE_8_5,
	OF_INDEXABLE_8_6,
	OF_INDEXABLE_8_7,
	OF_COMPILED_METHOD = 24,
	OF_COMPILED_METHOD_1,
	OF_COMPILED_METHOD_2,
	OF_COMPILED_METHOD_3,
	OF_COMPILED_METHOD_4,
	OF_COMPILED_METHOD_5,
	OF_COMPILED_METHOD_6,
	OF_COMPILED_METHOD_7,

	OF_INDEXABLE_NATIVE_FIRST = OF_INDEXABLE_64,
};

#define SLVM_SLOT_COUNT_OF_STRUCTURE(structure) \
    ((sizeof(structure) - sizeof(SLVM_ObjectHeader) + sizeof(SLVM_Oop) - 1) / sizeof(SLVM_Oop))

/**
 * The visible and fixed header of a dynamic object.
 **/
typedef struct SLVM_ObjectHeader_
{
    unsigned int classIndex : 22;
    unsigned int reserved : 2;
    unsigned int objectFormat : 5;
    unsigned int gcColor : 3;
    unsigned int identityHash : 22;
    unsigned int isPinned : 1;
    unsigned int isImmutable : 1;
    unsigned int slotCount : 8;
} SLVM_ObjectHeader;

typedef struct SLVM_Behavior_ SLVM_Behavior;

/**
 * Macros for building the object header.
 */
#define SLVM_PINNED_OBJECT_EMPTY_HEADER(classIndexValue, identityHashValue) { \
    .objectFormat = OF_EMPTY, \
    .classIndex = classIndexValue, \
    .identityHash = identityHashValue, \
    .isPinned = 1, \
    .slotCount = 0, \
}

#define SLVM_PINNED_OBJECT_FIXED_HEADER(slotCountValue, classIndexValue, identityHashValue) { \
    .objectFormat = OF_FIXED_SIZE, \
    .classIndex = classIndexValue, \
    .identityHash = identityHashValue, \
    .isPinned = 1, \
    .slotCount = slotCountValue, \
}

/**
 * Macros for identifying immediates vs pointer objects.
 */
#ifdef SLVM_SPUR_32
#define SLVM_SPUR_TAG_BITS 2
#define SLVM_SPUR_TAG_MASK 3

#define SLVM_SPUR_SMALLINTEGER_TAG_BITS 1
#define SLVM_SPUR_SMALLINTEGER_TAG_MASK 1
#define SLVM_SPUR_SMALLINTEGER_TAG_VALUE 1

#define SLVM_SPUR_CHARACTER_TAG_BITS 2
#define SLVM_SPUR_CHARACTER_TAG_MASK 3
#define SLVM_SPUR_CHARACTER_TAG_VALUE 2

#else /*SLVM_SPUR_OBJECT_MODEL_64*/
#define SLVM_SPUR_TAG_BITS 3
#define SLVM_SPUR_TAG_MASK 7

#define SLVM_SPUR_SMALLINTEGER_TAG_BITS SLVM_SPUR_TAG_BITS
#define SLVM_SPUR_SMALLINTEGER_TAG_MASK SLVM_SPUR_TAG_MASK
#define SLVM_SPUR_SMALLINTEGER_TAG_VALUE 1

#define SLVM_SPUR_CHARACTER_TAG_BITS SLVM_SPUR_TAG_BITS
#define SLVM_SPUR_CHARACTER_TAG_MASK SLVM_SPUR_TAG_MASK
#define SLVM_SPUR_CHARACTER_TAG_VALUE 2

#define SLVM_SPUR_SMALLFLOAT_TAG_BITS SLVM_SPUR_TAG_BITS
#define SLVM_SPUR_SMALLFLOAT_TAG_MASK SLVM_SPUR_TAG_MASK
#define SLVM_SPUR_SMALLFLOAT_TAG_VALUE 4
#endif

#define SLVM_SPUR_POINTER_TAG_BITS SLVM_SPUR_TAG_BITS
#define SLVM_SPUR_POINTER_TAG_MASK SLVM_SPUR_TAG_MASK
#define SLVM_SPUR_POINTER_TAG_VALUE 0

/**
 * Required object model testing and accessing macros.
 */
#define slvm_oopIsPointers(oop) (((oop) & SLVM_SPUR_POINTER_TAG_MASK) == SLVM_SPUR_POINTER_TAG_VALUE)
#define slvm_oopIsImmediate(oop) (((oop) & SLVM_SPUR_POINTER_TAG_MASK) != SLVM_SPUR_POINTER_TAG_VALUE)
#define slvm_oopIsSmallInteger(oop) (((oop) & SLVM_SPUR_SMALLINTEGER_TAG_MASK) == SLVM_SPUR_SMALLINTEGER_TAG_VALUE)
#define slvm_oopIsCharacter(oop) (((oop) & SLVM_SPUR_CHARACTER_TAG_MASK) == SLVM_SPUR_CHARACTER_TAG_VALUE)

#ifdef SLVM_SPUR_32
#define slvm_oopIsSmallFloat(oop) 0
#else
#define slvm_oopIsSmallFloat(oop) (((oop) & SLVM_SPUR_SMALLFLOAT_TAG_MASK) == SLVM_SPUR_SMALLFLOAT_TAG_VALUE)
#endif

#define slvm_encodeSmallIntegerOffset(value) (value << SLVM_SPUR_SMALLINTEGER_TAG_BITS)

#define slvm_encodeSmallInteger(value) ((value << SLVM_SPUR_SMALLINTEGER_TAG_BITS) | SLVM_SPUR_SMALLINTEGER_TAG_VALUE)
#define slvm_decodeSmallInteger(oop) (oop >> SLVM_SPUR_SMALLINTEGER_TAG_BITS)

#define slvm_getClassIndexFromOop(oop) (slvm_oopIsPointers(oop) ? ((SLVM_ObjectHeader*)(oop))->classIndex : ((oop) & SLVM_SPUR_TAG_MASK))
#define slvm_getClassFromOop(oop) slvm_classTable[slvm_getClassIndexFromOop(oop) >> 12][slvm_getClassIndexFromOop(oop) & 1023]

#define slvm_makeHashFromPointer(pointer) ((((SLVM_Oop) pointer) >> 4) & 0x3fffff)
#define slvm_identityHash(oop) (slvm_oopIsImmediate(oop) ? (oop >> SLVM_SPUR_SMALLINTEGER_TAG_BITS) : ((SLVM_ObjectHeader*)oop)->identityHash)

extern SLVM_Oop slvm_makeInteger(SLVM_Oop value);

/**
 * A sparse class table
 **/
extern SLVM_Behavior** slvm_classTable[4096];
extern void slvm_spur_initialize(void);
extern void slvm_spur_shutdown(void);

extern void slvm_objectmodel_registerBehavior(SLVM_Behavior *behavior);

extern size_t slvm_basicSize(SLVM_Oop object);

#endif /* SLVM_SPUR_OBJECT_MODEL_H */
