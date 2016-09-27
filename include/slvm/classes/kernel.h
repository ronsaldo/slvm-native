#ifndef SLVM_CLASSES_KERNEL_H
#define SLVM_CLASSES_KERNEL_H

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

#define slvm_behavior_encodeFormatAndFixedSize(format, fixedSlot) slvm_encodeSmallInteger(((format << 16) | fixedSlot))
#define slvm_behavior_decodeFormat(formatAndSize) (slvm_decodeSmallInteger(formatAndSize) >> 16)
#define slvm_behavior_decodeFixedSize(formatAndSize) (slvm_decodeSmallInteger(formatAndSize) & 0xFFFF)

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
                .format = slvm_behavior_encodeFormatAndFixedSize(formatType, fixedSize), \
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
                .format = slvm_behavior_encodeFormatAndFixedSize(OF_FIXED_SIZE, SLVM_SLOT_COUNT_OF_STRUCTURE(SLVM_Class)), \
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

#endif /* SLVM_CLASSES_KERNEL_H */
