#ifndef SLVM_LIFECYCLE_H
#define SLVM_LIFECYCLE_H

#include <stddef.h>

extern void slvm_dynrun_initialize(void);
extern void slvm_dynrun_shutdown(void);

/**
 * Registration of static heap with literal data
 */
extern void slvm_dynrun_registerStaticHeap(void *start, size_t size);
extern void slvm_dynrun_unregisterStaticHeap(void *start, size_t size);

#endif /* SLVM_LIFECYCLE_H */
