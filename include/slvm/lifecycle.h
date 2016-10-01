#ifndef SLVM_LIFECYCLE_H
#define SLVM_LIFECYCLE_H

#include <stddef.h>
#include "memory.h"

extern int slvm_dynrun_isKernelInitialized(void);

extern void slvm_dynrun_initialize(void);
extern void slvm_dynrun_shutdown(void);

#endif /* SLVM_LIFECYCLE_H */
