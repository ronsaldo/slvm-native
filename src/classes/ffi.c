#include "slvm/classes/ffi.h"
#include "slvm/classes/collections.h"

SLVM_IMPLEMENT_KERNEL_CLASS(ExternalAddress, ByteArray)
SLVM_IMPLEMENT_KERNEL_CLASS(ExternalObject, Object)
    SLVM_IMPLEMENT_KERNEL_CLASS(ExternalFunction, ExternalObject)
    SLVM_IMPLEMENT_KERNEL_CLASS(ExternalLibrary, ExternalObject)
    SLVM_IMPLEMENT_KERNEL_CLASS(ExternalStructure, ExternalObject)
        SLVM_IMPLEMENT_KERNEL_CLASS(ExternalData, ExternalStructure)

SLVM_IMPLEMENT_KERNEL_CLASS(Alien, ByteArray)
    SLVM_IMPLEMENT_KERNEL_CLASS(UnsafeAlien, Alien)
