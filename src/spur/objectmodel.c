#ifdef SLVM_SPUR_OBJECT_MODEL
#include "slvm/objectmodel.h"

/**
 * The global class table
 */
SLVM_Behavior** slvm_classTable[1024]

static void slvm_spur_initializeClassTable(void)
{
}

void slvm_spur_initialize(void)
{
    slvm_spur_initializeClassTable();
}

#endif /* SLVM_SPUR_OBJECT_MODEL */
