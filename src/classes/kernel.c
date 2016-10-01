#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "slvm/classes.h"
#include "slvm/memory.h"

/**
 * Kernel collections roots
 */
struct SLVM_KernelRoot
{
    /* Globals */
    SLVM_SystemDictionary *globals;
};

static struct SLVM_KernelRoot kernelRoots;

/**
 * Kernel class hierarchy
 */
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

/**
 * Special objects
 */
SLVM_UndefinedObject slvm_nil = {
    ._header_ = SLVM_PINNED_OBJECT_EMPTY_HEADER(SLVM_KCI_UndefinedObject, 0)
};

SLVM_False slvm_false = {
    ._header_ = SLVM_PINNED_OBJECT_EMPTY_HEADER(SLVM_KCI_False, 1)
};

SLVM_True slvm_true = {
    ._header_ = SLVM_PINNED_OBJECT_EMPTY_HEADER(SLVM_KCI_True, 2)
};

/**
 * Behavior primitives.
 */
SLVM_Oop slvm_Behavior_lookup(SLVM_Behavior *behavior, SLVM_Oop selector)
{
    SLVM_Behavior *current;
    SLVM_Oop value;
    SLVM_MethodDictionary *methodDictionary;
    SLVM_Oop methodDictionaryOop;

    /**
     * Lookup the selector up to the hierarchy chain.
     */
    for(current = behavior; !slvm_isNil(current); current = current->superclass)
    {
         methodDictionary = current->methodDict;

         /* Ignore nil method dictionaries. */
         if(slvm_isNil(current->methodDict))
            continue;

        /* Make sure this is actually a method dictionary. */
        methodDictionaryOop = (SLVM_Oop)methodDictionary;
        if(slvm_oopIsPointers(methodDictionaryOop) && slvm_getClassIndexFromOop(methodDictionaryOop) == SLVM_KCI_MethodDictionary)
        {
            value = slvm_MethodDictionary_atOrNil(current->methodDict, selector);
            if(!slvm_isNil(value))
                return value;
        }
        else
        {
            /* Method dictionary is a strange object. Send a special message to
              the dictionary to resolve the actual method. */
            fprintf(stderr, "Support for strange method dictionaries is not yet implemented.\n");
            abort();
        }
    }

    return slvm_nilOop;
}

/**
 * Kernel class initialization.
 */
void slvm_internal_init_classes(void)
{
    kernelRoots.globals = slvm_SystemDictionary_new();

    slvm_dynrun_registerArrayOfRoots((SLVM_Oop*)&kernelRoots, sizeof(kernelRoots) / sizeof(SLVM_Oop));

/**
 * Initialize the classes
 */
#define COMPACT_CLASS_ALIAS_INDEX(className, index)
#define COMPACT_CLASS_INDEX(className, index) \
    SLVM_KCLASS(className)->name = (SLVM_Oop)slvm_Symbol_internCString(#className); \
    SLVM_KCLASS(className)->classPool = (SLVM_Oop)slvm_Dictionary_new(SLVM_KCLASS(Dictionary)); \
    SLVM_KCLASS(className)->environment = (SLVM_Oop)kernelRoots.globals; \
    SLVM_KCLASS(className)->_base_._base_.methodDict = slvm_MethodDictionary_new(); \
    slvm_IdentityDictionary_atPut((SLVM_IdentityDictionary*)kernelRoots.globals, (SLVM_Oop)(SLVM_KCLASS(className)->name), (SLVM_Oop)SLVM_KCLASS(className));

    #include "slvm/classes/compactIndices.inc"

#undef COMPACT_CLASS_INDEX
#undef COMPACT_CLASS_ALIAS_INDEX
}
