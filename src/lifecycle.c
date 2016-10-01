#include "slvm/dynrun.h"

static int kernelInitialized = 0;

extern void slvm_internal_init_collections_kernel(void);
extern void slvm_internal_init_classes(void);

extern void slvm_internal_init_collections(void);
extern void slvm_internal_init_magnitude(void);

extern void slvm_internal_init_staticHeaps(void);

int slvm_dynrun_isKernelInitialized(void)
{
    return kernelInitialized;
}

void slvm_dynrun_initialize(void)
{
    /* Initialize the memory manager. */
#ifdef SLVM_SPUR_OBJECT_MODEL
    slvm_spur_initialize();
#else
#error TODO: implement myself
#endif

    /* Initialize the kernel */
    slvm_internal_init_collections_kernel();
    slvm_internal_init_classes();

    /* Initialize the main packages. */
    slvm_internal_init_collections();
    slvm_internal_init_magnitude();

    /* Now that we have initialized the kernel, lets initialize the user code. */
    kernelInitialized = 1;
    slvm_internal_init_staticHeaps();
}

void slvm_dynrun_shutdown(void)
{
#ifdef SLVM_SPUR_OBJECT_MODEL
    slvm_spur_shutdown();
#else
#error TODO: implement myself
#endif
}
