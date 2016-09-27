#include "slvm/objectmodel.h"
#include "slvm/classes.h"

#ifdef SLVM_SPUR_OBJECT_MODEL
static unsigned int slvm_classTableBaseSize = 0;
static unsigned int slvm_classTableSize = 0;

/**
 * The global class table
 */
SLVM_Behavior** slvm_classTable[4096];

/* The first page is special. It contains only "known" class indices. */
static SLVM_Behavior* slvm_classTableFirstPage[1024];

void slvm_spur_initialize(void)
{
    /* Initialize the first page with nil*/
    int i;
    for(i = 0; i < 1024; ++i)
        slvm_classTableFirstPage[i] = (SLVM_Behavior*)&slvm_nil;

    /* Add the compact class indices to the table. */
    #define COMPACT_CLASS_INDEX(className, index) \
        slvm_classTableFirstPage[index] = (SLVM_Behavior*)&slvm_kernel_class_ ## className; \
        slvm_classTableFirstPage[256 + index] = (SLVM_Behavior*)&slvm_kernel_metaclass_ ## className;
    #define COMPACT_CLASS_ALIAS_INDEX(className, index) COMPACT_CLASS_INDEX(className, index)

#include "slvm/classes/compactIndices.inc"

    #undef COMPACT_CLASS_INDEX
    #undef COMPACT_CLASS_ALIAS_INDEX

    /* Set the initial size of the table. */
    slvm_classTableBaseSize = 512;
    slvm_classTableSize = 0;
    slvm_classTable[0] = slvm_classTableFirstPage;
}

void slvm_spur_shutdown(void)
{
}

#endif /* SLVM_SPUR_OBJECT_MODEL */
