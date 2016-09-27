#ifndef SLVM_CLASSES_FFI_H
#define SLVM_CLASSES_FFI_H

#include "kernel.h"

typedef SLVM_Object SLVM_ExternalAddress;
typedef SLVM_Object SLVM_ExternalObject;
    typedef SLVM_ExternalObject SLVM_ExternalFunction;
    typedef SLVM_ExternalObject SLVM_ExternalLibrary;
    typedef SLVM_ExternalObject SLVM_ExternalStructure;
        typedef SLVM_ExternalStructure SLVM_ExternalData;
typedef SLVM_Object SLVM_Alien;
typedef SLVM_Object SLVM_UnsafeAlien;

SLVM_DECLARE_KERNEL_CLASS(ExternalAddress)
SLVM_DECLARE_KERNEL_CLASS(ExternalObject)
    SLVM_DECLARE_KERNEL_CLASS(ExternalFunction)
    SLVM_DECLARE_KERNEL_CLASS(ExternalLibrary)
    SLVM_DECLARE_KERNEL_CLASS(ExternalStructure)
    SLVM_DECLARE_KERNEL_CLASS(ExternalData)

SLVM_DECLARE_KERNEL_CLASS(Alien)
SLVM_DECLARE_KERNEL_CLASS(UnsafeAlien)

#endif /* SLVM_CLASSES_FFI_H */
