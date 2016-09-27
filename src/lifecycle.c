#include "slvm/dynrun.h"

void slvm_dynrun_initialize(void)
{
#ifdef SLVM_SPUR_OBJECT_MODEL
    slvm_spur_initialize();
#else
#error TODO: implement myself
#endif
}

void slvm_dynrun_shutdown(void)
{
#ifdef SLVM_SPUR_OBJECT_MODEL
    slvm_spur_shutdown();
#else
#error TODO: implement myself
#endif
}

void slvm_dynrun_registerStaticHeap(void *start, size_t size)
{
}

void slvm_dynrun_unregisterStaticHeap(void *start, size_t size)
{
}
