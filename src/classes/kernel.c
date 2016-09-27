#include "slvm/classes/kernel.h"

SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_SUPER_METACLASS(ProtoObject, (SLVM_Behavior*)&slvm_nil, (SLVM_Behavior*)&slvm_kernel_class_Class, OF_EMPTY, 0);
SLVM_IMPLEMENT_KERNEL_CLASS(Object, ProtoObject);

SLVM_IMPLEMENT_KERNEL_CLASS(Behavior, Object);
    SLVM_IMPLEMENT_KERNEL_CLASS(ClassDescription, Behavior);
        SLVM_IMPLEMENT_KERNEL_CLASS(Class, ClassDescription);
        SLVM_IMPLEMENT_KERNEL_CLASS(Metaclass, ClassDescription);

SLVM_IMPLEMENT_KERNEL_CLASS(UndefinedObject, Object);
SLVM_IMPLEMENT_KERNEL_CLASS(Boolean, Object);
    SLVM_IMPLEMENT_KERNEL_CLASS(True, Boolean);
    SLVM_IMPLEMENT_KERNEL_CLASS(False, Boolean);

SLVM_UndefinedObject slvm_nil = {
    ._header_ = SLVM_PINNED_OBJECT_EMPTY_HEADER(SLVM_KCI_UndefinedObject, 0)
};

SLVM_False slvm_false = {
    ._header_ = SLVM_PINNED_OBJECT_EMPTY_HEADER(SLVM_KCI_False, 1)
};

SLVM_True slvm_true = {
    ._header_ = SLVM_PINNED_OBJECT_EMPTY_HEADER(SLVM_KCI_True, 2)
};
