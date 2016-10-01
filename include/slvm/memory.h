#ifndef SLVM_MEMORY_H
#define SLVM_MEMORY_H

#include <stddef.h>
#include "objectmodel.h"
#include "datastructures.h"

enum SLVM_HeapFlags
{
    SHF_CannotBeMove = 1<<0,
    SHF_MayNeedFixingUp = 1<<1,
    SHF_Initialized = 1<<2,
};

typedef struct SLVM_HeapInformation_ SLVM_HeapInformation;

struct SLVM_HeapInformation_
{
    SLVM_LinkedListNode header;
    size_t size;
    size_t flags;
    void *start;
};

/**
 * Regstration of array of roots
 */
extern void slvm_dynrun_registerArrayOfRoots(SLVM_Oop *array, size_t numberOfElements);
extern void slvm_dynrun_unregisterArrayOfRoots(SLVM_Oop *array, size_t numberOfElements);

/**
 * Registration of static heap with literal data
 */
extern void slvm_dynrun_registerStaticHeap(SLVM_HeapInformation *heapInformation);
extern void slvm_dynrun_unregisterStaticHeap(SLVM_HeapInformation *heapInformation);

#endif /* SLVM_MEMORY_H */
