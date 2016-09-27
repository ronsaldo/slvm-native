#ifndef SLVM_CLASSES_KERNEL_H
#define SLVM_CLASSES_KERNEL_H

#include "../objectmodel.h"

#define SLVM_DECLARE_KERNEL_CLASS(className) \
extern SLVM_Class slvm_kernel_class_ ## className ; \
extern SLVM_Class slvm_kernel_metaclass_ ## className ;

typedef struct SLVM_Object_ SLVM_ProtoObject;
typedef struct SLVM_Object_ SLVM_Object;
typedef struct SLVM_Behavior_ SLVM_Behavior;
typedef struct SLVM_ClassDescription_ SLVM_ClassDescription;
typedef struct SLVM_Class_ SLVM_Class;
typedef struct SLVM_Metaclass_ SLVM_Metaclass;

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
};

/**
 * ClassDescription
 */
struct SLVM_ClassDescription_
{
    SLVM_Behavior _base_;
};

/**
 * Class
 */
struct SLVM_Class_
{
    SLVM_ClassDescription _base_;
};

/**
 * Metaclass
 */
struct SLVM_Metaclass_
{
    SLVM_ClassDescription _base_;
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

#endif /* SLVM_CLASSES_KERNEL_H */
