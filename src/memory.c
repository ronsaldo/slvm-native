#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include "slvm/memory.h"

static SLVM_StackHeapInformation executionStackMemoryPool;
static SLVM_LinkedList executionStackFreeList;
static __thread SLVM_ThreadStackData *currentThreadStackData;

void slvm_ExecutionStack_initialize(void)
{
    int result = slvm_StackHeap_create(&executionStackMemoryPool, SLVM_EXECUTION_STACK_CAPACITY, SMPF_Readable|SMPF_Writeable);
    if(result != 0)
    {
        fprintf(stderr, "Failed to allocate the execution stack virtual address space.\n");
        abort();
    }
}

SLVM_ExecutionStackSegmentHeader* slvm_ExecutionStack_getFirstSegment(void)
{
    return currentThreadStackData ? (SLVM_ExecutionStackSegmentHeader*)currentThreadStackData->segmentList.first : 0;
}

SLVM_ExecutionStackSegmentHeader* slvm_ExecutionStack_getLastSegment(void)
{
    return currentThreadStackData ? (SLVM_ExecutionStackSegmentHeader*)currentThreadStackData->segmentList.last : 0;
}

SLVM_ExecutionStackSegmentHeader* slvm_ExecutionStack_allocateSegment(void)
{
    uint8_t *end;
    SLVM_ExecutionStackSegmentHeader* result;

    /* Allocate the segment. */
    if(executionStackFreeList.first)
    {
        result = (SLVM_ExecutionStackSegmentHeader*) ((uint8_t*)executionStackFreeList.first + sizeof(SLVM_LinkedListNode) - sizeof(SLVM_ExecutionStackSegmentHeader));
        slvm_list_removeNode(&executionStackFreeList, executionStackFreeList.first);
    }
    else
    {
        end = (uint8_t*)slvm_StackHeap_allocate(&executionStackMemoryPool, SLVM_STACK_SEGMENT_NORMAL_SIZE, SLVM_STACK_SEGMENT_NORMAL_SIZE);
        assert((sizeof(SLVM_ExecutionStackSegmentHeader) & 15) == 0);
        result = (SLVM_ExecutionStackSegmentHeader*) (end + SLVM_STACK_SEGMENT_NORMAL_SIZE - sizeof(SLVM_ExecutionStackSegmentHeader));
    }

    /* Clear the segment*/
    memset(result, 0, sizeof(SLVM_ExecutionStackSegmentHeader));

    /* Set the initial stack pointer and frame pointer*/
    result->stackPointer = (uint8_t*)result;
    return result;
}

SLVM_ExecutionStackSegmentHeader* slvm_ExecutionStack_getValidSegment(void)
{
    SLVM_ExecutionStackSegmentHeader *result;

    if(slvm_ExecutionStack_getLastSegment())
        return slvm_ExecutionStack_getLastSegment();

    /* Allocate a new segment*/
    result = slvm_ExecutionStack_allocateSegment();
    if(!slvm_ExecutionStack_getFirstSegment())
    {
        /* Allocate the thread stack data*/
        result->stackPointer -= (sizeof(SLVM_ThreadStackData) + 15) & (-16);
        currentThreadStackData = result->threadData = (SLVM_ThreadStackData*)result->stackPointer;
        memset(currentThreadStackData, 0, sizeof(SLVM_ThreadStackData));
    }
    else
    {
        assert(currentThreadStackData);
        result->threadData = currentThreadStackData;
    }

    slvm_list_addNode(&currentThreadStackData->segmentList, &result->header);
    return result;
}

SLVM_ExecutionStackSegmentHeader* slvm_ExecutionStack_createNewSegment(void)
{
    SLVM_ExecutionStackSegmentHeader *result;
    assert(currentThreadStackData);

    result = slvm_ExecutionStack_allocateSegment();
    result->threadData = currentThreadStackData;
    slvm_list_addNode(&currentThreadStackData->segmentList, &result->header);
    return result;
}

void slvm_ExecutionStack_popSegment(SLVM_ExecutionStackSegmentHeader* segment)
{
    slvm_list_removeNode(&currentThreadStackData->segmentList, &segment->header);
    if(!currentThreadStackData->segmentList.first)
        currentThreadStackData = 0;

    slvm_list_addNode(&executionStackFreeList, &segment->header);
}

int slvm_StackHeap_create(SLVM_StackHeapInformation *result, size_t size, unsigned int permissions)
{
    memset(result, 0, sizeof(SLVM_StackHeapInformation));
    result->capacity = size;
    result->permissions = permissions;

    /* Allocate the address space */
    result->base.start = slvm_vm_allocateSpace(size);
    if(!result->base.start)
        return -1;

    return 0;
}

void slvm_StackHeap_destroy(SLVM_StackHeapInformation *heap)
{
    assert(heap);
    assert(heap->base.start);

    /* Deallocate the address space. */
    slvm_vm_freeSpace(heap->base.start, heap->capacity);
}

void *slvm_StackHeap_allocate(SLVM_StackHeapInformation *heap, size_t alignment, size_t size)
{
    size_t requiredCommitedCapacity;
    size_t offset;
    size_t newSize;

    assert(heap);
    assert(heap->base.start);

    /* Align the size. */
    offset = (heap->base.size + alignment - 1) & (-alignment);

    /* Compute the new size. */
    newSize = offset + size;

    /* Compute the required commited capacity. */
    requiredCommitedCapacity = (newSize + SLVM_PAGE_SIZE - 1) & (-SLVM_PAGE_SIZE);

    /* Commit the missing capacity. */
    if(heap->commitedCapacity < requiredCommitedCapacity)
    {
        slvm_vm_commitRange(heap->base.start, heap->commitedCapacity, requiredCommitedCapacity - heap->commitedCapacity, heap->permissions);
        heap->commitedCapacity = requiredCommitedCapacity;
    }

    /* Update the heap size and return. */
    heap->base.size = newSize;
    return (uint8_t*)heap->base.start + offset;
}
