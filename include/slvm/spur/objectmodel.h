#ifndef SLVM_SPUR_OBJECT_MODEL_H
#define SLVM_SPUR_OBJECT_MODEL_H

#include <stdint.h>

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

#define slvm_getClassIndexFromOop(oop) (slvm_oopIsPointers(oop) ? ((SLVM_ObjectHeader*)(oop))->classIndex : ((oop) & SLVM_SPUR_TAG_MASK))
#define slvm_getClassFromOop(oop) slvm_classTable[slvm_getClassIndexFromOop(oop)]

/**
 * A sparse class table
 **/
extern SLVM_Behavior** slvm_classTable[1024];
extern void slvm_spur_initialize(void);

#endif /* SLVM_SPUR_OBJECT_MODEL_H */
