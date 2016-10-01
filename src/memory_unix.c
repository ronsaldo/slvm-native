#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include "slvm/memory.h"

void *slvm_vm_allocateSpace(size_t size)
{
    void *result = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(result == MAP_FAILED)
    {
        fprintf(stderr, "Allocation of address space size %zu failed.\n", size);
        return NULL;
    }

    return result;
}

void slvm_vm_freeSpace(void *start, size_t size)
{
    munmap(start, size);
}

void slvm_vm_commitRange(void* start, size_t offset, size_t size, int permissions)
{
    slvm_vm_changeRangePermissions(start, offset, size, permissions);
}

void slvm_vm_changeRangePermissions(void* start, size_t offset, size_t size, int permissions)
{
    int result;
    int mappedPermissions = PROT_NONE;
    if(permissions & SMPF_Readable)
        mappedPermissions |= PROT_READ;
    if(permissions & SMPF_Writeable)
        mappedPermissions |= PROT_WRITE;
    if(permissions & SMPF_Executable)
        mappedPermissions |= PROT_EXEC;

    result = mprotect((uint8_t*)start + offset, size, mappedPermissions);
    if(result != 0)
        fprintf(stderr, "VM range permission change failed: %d\n", errno);
}

void slvm_vm_releaseRange(void* start, size_t offset, size_t size)
{
    int result = mprotect((uint8_t*)start + offset, size, PROT_NONE);
    if(result != 0)
        fprintf(stderr, "VM range release failed: %d\n", errno);
}
