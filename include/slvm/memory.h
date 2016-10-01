#ifndef SLVM_MEMORY_H
#define SLVM_MEMORY_H

#include <stddef.h>
#include "objectmodel.h"

/**
 * Regstration of array of roots
 */
extern void slvm_dynrun_registerArrayOfRoots(SLVM_Oop *array, size_t numberOfElements);
extern void slvm_dynrun_unregisterArrayOfRoots(SLVM_Oop *array, size_t numberOfElements);

/**
 * Registration of static heap with literal data
 */
extern void slvm_dynrun_registerStaticHeap(void *start, size_t size);
extern void slvm_dynrun_unregisterStaticHeap(void *start, size_t size);

#endif /* SLVM_MEMORY_H */
