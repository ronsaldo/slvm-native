#ifndef SLVM_MEMORY_H
#define SLVM_MEMORY_H

#include <stddef.h>
#include "objectmodel.h"
#include "datastructures.h"

#define SLVM_PAGE_SIZE 4096

enum SLVM_HeapFlags
{
    SHF_CannotBeMove = 1<<0,
    SHF_MayNeedFixingUp = 1<<1,
    SHF_Initialized = 1<<2,
};

enum SLVM_MemoryPermissionFlags
{
    SMPF_Readable = 1<<0,
    SMPF_Writeable = 1<<1,
    SMPF_Executable = 1<<2,
};

typedef struct SLVM_HeapInformation_ SLVM_HeapInformation;
typedef struct SLVM_StackHeapInformation_ SLVM_StackHeapInformation;

struct SLVM_HeapInformation_
{
    SLVM_LinkedListNode header;
    size_t size;
    size_t flags;
    void *start;
};

struct SLVM_StackHeapInformation_
{
    SLVM_HeapInformation base;
    size_t capacity;
    size_t commitedCapacity;
    unsigned int permissions;
};

/**
 * Virtual memory interface
 */
extern void *slvm_vm_allocateSpace(size_t size);
extern void slvm_vm_freeSpace(void *start, size_t size);

extern void slvm_vm_commitRange(void* start, size_t offset, size_t size, int permissions);
extern void slvm_vm_changeRangePermissions(void* start, size_t offset, size_t size, int permissions);
extern void slvm_vm_releaseRange(void* start, size_t offset, size_t size);

/**
 * Stack allocation heap
 */
extern int slvm_StackHeap_create(SLVM_StackHeapInformation *heap, size_t size, unsigned int permissions);
extern void slvm_StackHeap_destroy(SLVM_StackHeapInformation *heap);
extern void *slvm_StackHeap_allocate(SLVM_StackHeapInformation *heap, size_t alignment, size_t size);

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
