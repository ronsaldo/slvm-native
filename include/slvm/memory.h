#ifndef SLVM_MEMORY_H
#define SLVM_MEMORY_H

#include <stddef.h>
#include "objectmodel.h"
#include "datastructures.h"

#define SLVM_PAGE_SIZE 4096
#define SLVM_STACK_SEGMENT_NORMAL_SIZE SLVM_PAGE_SIZE
#define SLVM_EXECUTION_STACK_CAPACITY (64 << 20) /* 64 MB*/

enum SLVM_HeapFlags
{
    SHF_CannotBeMove = 1<<0,
    SHF_MayNeedFixingUp = 1<<1,
    SHF_HasPackageRegistration = 1<<2,
    SHF_ExecutionStack = 1<<3,
    SHF_Initialized = 1<<10,
};

enum SLVM_MemoryPermissionFlags
{
    SMPF_Readable = 1<<0,
    SMPF_Writeable = 1<<1,
    SMPF_Executable = 1<<2,
};

typedef void (*SLVM_PackageRegistrationFunction) (void);

typedef struct SLVM_HeapInformation_ SLVM_HeapInformation;
typedef struct SLVM_HeapWithPackageInformation_ SLVM_HeapWithPackageInformation;
typedef struct SLVM_StackHeapInformation_ SLVM_StackHeapInformation;
typedef struct SLVM_ExecutionStackSegmentHeader_ SLVM_ExecutionStackSegmentHeader;
typedef struct SLVM_ThreadStackData_ SLVM_ThreadStackData;

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

struct SLVM_HeapWithPackageInformation_
{
    SLVM_HeapInformation base;
    SLVM_PackageRegistrationFunction packageRegistration;
};

struct SLVM_ThreadStackData_
{
    uint8_t *cstackPointer;
    uint8_t *cframePointer;
    uint8_t *creturnPointer;
    uint8_t *cextraPointer;
    SLVM_LinkedList segmentList;
};

struct SLVM_ExecutionStackSegmentHeader_
{
    void* reserved;
    size_t segmentSize;
    SLVM_ThreadStackData *threadData;
    uint8_t *linkPointer; /* Not used in X86 */
    uint8_t *framePointer;
    uint8_t *stackPointer;
    SLVM_LinkedListNode header;
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
 * Segmented execution stack
 */
extern void slvm_ExecutionStack_initialize(void);
extern SLVM_ExecutionStackSegmentHeader* slvm_ExecutionStack_getFirstSegment(void);
extern SLVM_ExecutionStackSegmentHeader* slvm_ExecutionStack_getLastSegment(void);
extern SLVM_ExecutionStackSegmentHeader* slvm_ExecutionStack_getValidSegment(void);
extern SLVM_ExecutionStackSegmentHeader* slvm_ExecutionStack_allocateSegment(void);
extern SLVM_ExecutionStackSegmentHeader* slvm_ExecutionStack_createNewSegment(void);
extern void slvm_ExecutionStack_popSegment(SLVM_ExecutionStackSegmentHeader* segment);

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
