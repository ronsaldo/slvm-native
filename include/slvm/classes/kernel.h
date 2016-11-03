#ifndef SLVM_CLASSES_KERNEL_H
#define SLVM_CLASSES_KERNEL_H

#include <stddef.h>
#include "../objectmodel.h"
#include "compactIndices.h"

#define SLVM_DECLARE_KERNEL_CLASS(className) \
extern SLVM_Class slvm_kernel_class_ ## className ; \
extern SLVM_Metaclass slvm_kernel_metaclass_ ## className ;

typedef struct SLVM_Object_ SLVM_ProtoObject;
typedef struct SLVM_Object_ SLVM_Object;
typedef struct SLVM_Behavior_ SLVM_Behavior;
typedef struct SLVM_ClassDescription_ SLVM_ClassDescription;
typedef struct SLVM_Class_ SLVM_Class;
typedef struct SLVM_Metaclass_ SLVM_Metaclass;
typedef struct SLVM_MethodDictionary_ SLVM_MethodDictionary;

/**
 * Special empty objects
 */
typedef SLVM_Object SLVM_Boolean;
typedef SLVM_Object SLVM_False;
typedef SLVM_Object SLVM_True;
typedef SLVM_Object SLVM_UndefinedObject;

/**
 * Object
 */
struct SLVM_Object_
{
    SLVM_ObjectHeader _header_;
};

/**
 * Behavior
 */
struct SLVM_Behavior_
{
    SLVM_Object _base_;
    SLVM_Behavior *superclass;
    SLVM_MethodDictionary *methodDict;
    SLVM_SOop format;
    SLVM_Oop layout;
};

/**
 * ClassDescription
 */
struct SLVM_ClassDescription_
{
    SLVM_Behavior _base_;
    SLVM_Oop instanceVariables;
    SLVM_Oop organization;
};

/**
 * Class
 */
struct SLVM_Class_
{
    SLVM_ClassDescription _base_;
    SLVM_Oop subclasses;
    SLVM_Oop name;
    SLVM_Oop classPool;
    SLVM_Oop sharedPools;
    SLVM_Oop environment;
    SLVM_Oop category;
    SLVM_Oop traitComposition;
    SLVM_Oop localSelectors;
};

/**
 * Metaclass
 */
struct SLVM_Metaclass_
{
    SLVM_ClassDescription _base_;
    SLVM_Class *thisClass;
    SLVM_Oop traitComposition;
    SLVM_Oop localSelectors;
};

/**
 * Kernel classes
 */
SLVM_DECLARE_KERNEL_CLASS(ProtoObject);
SLVM_DECLARE_KERNEL_CLASS(Object);

SLVM_DECLARE_KERNEL_CLASS(Behavior);
    SLVM_DECLARE_KERNEL_CLASS(ClassDescription);
        SLVM_DECLARE_KERNEL_CLASS(Class);
        SLVM_DECLARE_KERNEL_CLASS(Metaclass);

SLVM_DECLARE_KERNEL_CLASS(UndefinedObject);
SLVM_DECLARE_KERNEL_CLASS(Boolean);
    SLVM_DECLARE_KERNEL_CLASS(True);
    SLVM_DECLARE_KERNEL_CLASS(False);

/**
 * Special kernel objects
 */
extern SLVM_UndefinedObject slvm_nil;
extern SLVM_True slvm_true;
extern SLVM_False slvm_false;

#define slvm_nilOop ((SLVM_Oop)&slvm_nil)
#define slvm_falseOop ((SLVM_Oop)&slvm_false)
#define slvm_trueOop ((SLVM_Oop)&slvm_true)
#define slvm_isNil(object) ((SLVM_Oop)object == slvm_nilOop)

#define slvm_encodeBoolean(expr) ((expr) ? slvm_trueOop : slvm_falseOop)
/**
 * Behavior methods
 */
#define slvm_Behavior_encodeFormatAndFixedSize(format, fixedSlot) slvm_encodeSmallInteger(((format << 16) | fixedSlot))
#define slvm_Behavior_decodeFormat(formatAndSize) (slvm_decodeSmallInteger(formatAndSize) >> 16)
#define slvm_Behavior_decodeFixedSize(formatAndSize) (slvm_decodeSmallInteger(formatAndSize) & 0xFFFF)

extern SLVM_ProtoObject *slvm_Behavior_basicNew(SLVM_Behavior *behavior, size_t variableSize);
extern SLVM_Oop slvm_Behavior_lookup(SLVM_Behavior *behavior, SLVM_Oop selector);

/**
 * Global dictionary
 */
extern SLVM_Oop slvm_globals_atOrNil(SLVM_Oop symbol);
extern void slvm_globals_atPut(SLVM_Oop symbol, SLVM_Oop value);

#define SLVM_GET_GLOBAL(globalName) \
    slvm_globals_atOrNil((SLVM_Oop)slvm_Symbol_internCString(#globalName))

#define SLVM_SET_GLOBAL(globalName, value) \
    slvm_globals_atPut((SLVM_Oop)slvm_Symbol_internCString(#globalName), (SLVM_Oop)(value))

/**
 * Implementation of the kernel classes.
 */
#define SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_SUPER_METACLASS(className, superClassValue, superMetaClassValue, formatType, fixedSize) \
    SLVM_Class slvm_kernel_class_ ## className = { \
        ._base_ = { \
            ._base_ = { \
                ._base_ = { \
                    ._header_ = SLVM_PINNED_OBJECT_FIXED_HEADER(SLVM_SLOT_COUNT_OF_STRUCTURE(SLVM_Class), SLVM_KMCI_ ## className, SLVM_KCI_ ## className), \
                }, \
                /* Behavior */  \
                .superclass = superClassValue, \
                .methodDict = (SLVM_MethodDictionary*)&slvm_nil, \
                .format = slvm_Behavior_encodeFormatAndFixedSize(formatType, fixedSize), \
                .layout = slvm_nilOop, \
            }, \
            /* Class Description*/  \
            .instanceVariables = slvm_nilOop, \
            .organization = slvm_nilOop, \
        }, \
        .subclasses = slvm_nilOop, \
        .name = slvm_nilOop, \
        .classPool = slvm_nilOop, \
        .sharedPools = slvm_nilOop, \
        .environment = slvm_nilOop, \
        .category = slvm_nilOop, \
        .traitComposition = slvm_nilOop, \
        .localSelectors = slvm_nilOop, \
    }; \
    SLVM_Metaclass slvm_kernel_metaclass_ ## className = { \
        ._base_ = { \
            ._base_ = { \
                ._base_ = { \
                    ._header_ = SLVM_PINNED_OBJECT_FIXED_HEADER(SLVM_SLOT_COUNT_OF_STRUCTURE(SLVM_Metaclass), SLVM_KCI_Metaclass, SLVM_KMCI_ ## className), \
                }, \
                /* Behavior */  \
                .superclass = superMetaClassValue, \
                .methodDict = (SLVM_MethodDictionary*)&slvm_nil, \
                .format = slvm_Behavior_encodeFormatAndFixedSize(OF_FIXED_SIZE, SLVM_SLOT_COUNT_OF_STRUCTURE(SLVM_Class)), \
                .layout = slvm_nilOop, \
            }, \
            /* Class Description*/  \
            .instanceVariables = slvm_nilOop, \
            .organization = slvm_nilOop, \
        }, \
        .thisClass = &slvm_kernel_class_ ## className, \
        .traitComposition = slvm_nilOop, \
        .localSelectors = slvm_nilOop, \
    }; \

#define SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(className, superClassName, formatType, fixedSize) \
SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_SUPER_METACLASS(className, \
    (SLVM_Behavior*)&slvm_kernel_class_ ## superClassName, \
    (SLVM_Behavior*)&slvm_kernel_metaclass_ ## superClassName, \
    formatType, fixedSize)

#define SLVM_IMPLEMENT_KERNEL_CLASS(className, superClassName) \
SLVM_IMPLEMENT_KERNEL_CLASS_EXPLICIT_FORMAT(className, superClassName, \
    (SLVM_SLOT_COUNT_OF_STRUCTURE(SLVM_ ## className) > 0 ? OF_FIXED_SIZE : OF_EMPTY), \
    SLVM_SLOT_COUNT_OF_STRUCTURE(SLVM_ ## className))


#define SLVM_KCLASS(className) \
    (&slvm_kernel_class_ ## className)

#define SLVM_KCLASS_DESCRIPTION(className) \
    ((SLVM_ClassDescription*)SLVM_KCLASS(className))

#define SLVM_KCLASS_BEHAVIOR(className) \
    ((SLVM_Behavior*)SLVM_KCLASS(className))

#define SLVM_KNEW(className, extraSize) \
    (SLVM_ ## className*) slvm_Behavior_basicNew((SLVM_Behavior*)SLVM_KCLASS(className), extraSize)

#define SLVM_KCLASS_VARIABLE_SET(className, classVariableName, classVariableValue)  \
    slvm_IdentityDictionary_atPut((SLVM_IdentityDictionary*)SLVM_KCLASS(className)->classPool, \
        (SLVM_Oop)slvm_Symbol_internCString(#classVariableName), (SLVM_Oop)(classVariableValue))

#define SLVM_KCLASS_ADD_PRIMITIVE(className, selector, primitiveNameSuffix) \
    slvm_MethodDictionary_atPut(SLVM_KCLASS_BEHAVIOR(className)->methodDict, \
        (SLVM_Oop)slvm_Symbol_internCString(selector), \
        (SLVM_Oop)slvm_PrimitiveMethod_make(&slvm_ ## className ## _primitive_ ## primitiveNameSuffix));

#define SLVM_SELECTOR(value) ((SLVM_Oop)slvm_Symbol_internCString(value))
/**
 * Class and method registration.
 */
extern SLVM_Oop slvm_dynrun_subclassWithSomeNames(SLVM_Oop superClassName,
        SLVM_Oop name,
        SLVM_Oop instanceVariableNames, SLVM_Oop format,
        SLVM_Oop metaInstanceVariableNames, SLVM_Oop metaFormat,
        SLVM_Oop classVariableAssociations, SLVM_Oop poolDictionaries,
        SLVM_Oop categoryName);
extern SLVM_Oop slvm_dynrun_registerMethodWithNames(SLVM_Oop method, SLVM_Oop selector, SLVM_Oop className, SLVM_Oop classSide);

/**
 * Message send
 */
extern void slvm_dynrun_send(void *stackPointer);
extern SLVM_Oop slvm_dynrun_csend(int argumentDescription, SLVM_Oop selector, ...);

#endif /* SLVM_CLASSES_KERNEL_H */
