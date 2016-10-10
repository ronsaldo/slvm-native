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
 * Global dictionary
 */
SLVM_Oop slvm_globals_atOrNil(SLVM_Oop symbol)
{
    return slvm_IdentityDictionary_atOrNil((SLVM_IdentityDictionary*)kernelRoots.globals, symbol);
}

void slvm_globals_atPut(SLVM_Oop symbol, SLVM_Oop value)
{
    slvm_IdentityDictionary_atPut((SLVM_IdentityDictionary*)kernelRoots.globals, symbol, value);
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

/**
 * Package registration methods.
 */
extern SLVM_Oop slvm_dynrun_subclassWithNames(SLVM_Oop superClassName,
        SLVM_Oop name,
        SLVM_Oop instanceVariableNames, SLVM_Oop format,
        SLVM_Oop metaInstanceVariableNames, SLVM_Oop metaFormat,
        SLVM_Oop classVariablesNames, SLVM_Oop poolDictionaries,
        SLVM_Oop categoryName)
{
    SLVM_Oop existing;
    SLVM_Oop superClassOop;
    SLVM_Metaclass *superMetaclass;
    SLVM_Class *superClass;

    SLVM_Metaclass *metaClass;
    SLVM_Behavior *metaClassBehavior;
    SLVM_ClassDescription *metaClassDescription;

    SLVM_Class *clazz;
    SLVM_Behavior *classBehavior;
    SLVM_ClassDescription *classDescription;

    /*printf("Register class: ");
    slvm_String_printLine((SLVM_String*)name);*/

    /* Try to find an existing version*/
    existing = slvm_IdentityDictionary_atOrNil((SLVM_IdentityDictionary*)kernelRoots.globals, name);
    if(!slvm_isNil(existing))
        return existing;

    /* Fetch the super class */
    superClassOop = slvm_IdentityDictionary_atOrNil((SLVM_IdentityDictionary*)kernelRoots.globals, superClassName);
    if(slvm_isNil(superClassOop) || slvm_oopIsImmediate(superClassOop))
    {
        fprintf(stderr, "Fatal Error: Failed to find super class on package registration: ");
        slvm_String_printLine((SLVM_String*)superClassName);
        abort();
    }

    /* Fetch the meta class */
    superClass = (SLVM_Class*)superClassOop;
    superMetaclass = (SLVM_Metaclass*)slvm_getClassFromOop(superClassOop);

    /* Create the metaclass */
    metaClass = SLVM_KNEW(Metaclass, 0);

    metaClassBehavior = (SLVM_Behavior*)metaClass;
    slvm_objectmodel_registerBehavior(metaClassBehavior);

    metaClassBehavior->format = metaFormat;
    metaClassBehavior->methodDict = slvm_MethodDictionary_new();

    metaClassDescription = (SLVM_ClassDescription*)metaClass;
    metaClassDescription->instanceVariables = metaInstanceVariableNames;

    /* Create the class */
    classBehavior = (SLVM_Behavior*)slvm_Behavior_basicNew(metaClassBehavior, 0);
    assert(slvm_getClassFromOop((SLVM_Oop)classBehavior) == metaClassBehavior);
    slvm_objectmodel_registerBehavior(classBehavior);

    classBehavior->format = format;
    classBehavior->methodDict = slvm_MethodDictionary_new();

    classDescription = (SLVM_ClassDescription*)classBehavior;
    classDescription->instanceVariables = instanceVariableNames;

    clazz = (SLVM_Class*)classBehavior;
    clazz->category = categoryName;
    clazz->name = name;

    /* Link the metaclass with its class */
    metaClass->thisClass = clazz;

    /* Register the class in the system dictionary. */
    slvm_IdentityDictionary_atPut((SLVM_IdentityDictionary*)kernelRoots.globals, clazz->name, (SLVM_Oop)clazz);
    return (SLVM_Oop)metaClass;
}

extern SLVM_Oop slvm_dynrun_registerMethodWithNames(SLVM_Oop method, SLVM_Oop selector, SLVM_Oop className, SLVM_Oop classSide)
{
    SLVM_Oop classOop;
    SLVM_Behavior *clazz;
    SLVM_Behavior *metaclass;

    /* Find the class. */
    classOop = slvm_IdentityDictionary_atOrNil((SLVM_IdentityDictionary*)kernelRoots.globals, className);
    /*printf("Method flags: %p\n", (void*)((SLVM_CompiledMethod*)method)->flags);
    printf("Register selector: ");
    slvm_String_printLine((SLVM_String*)selector);

    printf("Find method class %p %d\n", (void*)classOop, slvm_isNil(classOop));
    printf("ClassName %p:", (void*)className);
    slvm_String_printLine((SLVM_String*)className);*/

    if(slvm_isNil(classOop))
        return slvm_falseOop;

    /* Fetch the metaclass */
    metaclass = slvm_getClassFromOop(classOop);
    assert(metaclass->_base_._header_.classIndex == SLVM_KCI_Metaclass);

    /* Register the method in the method dictionary. */
    if(classSide == slvm_trueOop)
    {
        /* printf("Register in class side.\n"); */
        slvm_MethodDictionary_atPut(metaclass->methodDict, selector, method);
    }
    else
    {
        clazz = (SLVM_Behavior*)classOop;
        /* printf("Register in instance side.\n"); */
        slvm_MethodDictionary_atPut(clazz->methodDict, selector, method);
    }

    return slvm_trueOop;
}

/**
 * ProtoObject primitives
 */
static SLVM_Oop slvm_ProtoObject_primitive_asString(SLVM_PrimitiveContext *context)
{
    SLVM_Behavior *behavior = slvm_getClassFromOop(context->receiver);
    unsigned int metaclassIndex = slvm_getClassIndexFromOop((SLVM_Oop)behavior);
    if(metaclassIndex == SLVM_KCI_Metaclass)
        return ((SLVM_Metaclass*)behavior)->thisClass->name;
    else
        return ((SLVM_Class*)behavior)->name;
}

static SLVM_Oop slvm_ProtoObject_primitive_basicSize(SLVM_PrimitiveContext *context)
{
    return slvm_encodeSmallInteger(slvm_basicSize(context->receiver));
}

static SLVM_Oop slvm_ProtoObject_primitive_class(SLVM_PrimitiveContext *context)
{
    return (SLVM_Oop)slvm_getClassFromOop(context->receiver);
}

/**
 * Behavior primitives
 */
static SLVM_Oop slvm_Behavior_primitive_basicNew(SLVM_PrimitiveContext *context)
{
    SLVM_Behavior *behavior = (SLVM_Behavior*)context->receiver;
    size_t variableSize = 0;

    if(context->oopArgumentCount == 1)
    {
        if(!slvm_oopIsSmallInteger(context->oopArguments[0]))
            slvm_primitiveFailWithError(context, SLVM_PrimitiveError_InvalidArgument);

        variableSize = slvm_decodeSmallInteger(context->oopArguments[0]);
    }

    return (SLVM_Oop)slvm_Behavior_basicNew(behavior, variableSize);
}

static SLVM_Oop slvm_Behavior_primitive_registerAsBehavior(SLVM_PrimitiveContext *context)
{
    slvm_objectmodel_registerBehavior((SLVM_Behavior*)context->receiver);
    return context->receiver;
}

void slvm_internal_init_kernel(void)
{
    /* ProtoObject */
    SLVM_KCLASS_ADD_PRIMITIVE(ProtoObject, "asString", asString);
    SLVM_KCLASS_ADD_PRIMITIVE(ProtoObject, "class", class);
    SLVM_KCLASS_ADD_PRIMITIVE(ProtoObject, "basicSize", basicSize);

    /* Behavior */
    SLVM_KCLASS_ADD_PRIMITIVE(Behavior, "basicNew", basicNew);
    SLVM_KCLASS_ADD_PRIMITIVE(Behavior, "registerAsBehavior", registerAsBehavior);
}
