#include <assert.h>
#include <string.h>
#include <malloc.h>
#include "slvm/memory.h"

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
