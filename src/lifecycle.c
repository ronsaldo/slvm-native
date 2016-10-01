#include "slvm/dynrun.h"

extern void slvm_internal_init_collections_kernel(void);
extern void slvm_internal_init_collections(void);
extern void slvm_internal_init_classes(void);

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

    /* Initialize the collections */
    slvm_internal_init_collections();

}

void slvm_dynrun_shutdown(void)
{
#ifdef SLVM_SPUR_OBJECT_MODEL
    slvm_spur_shutdown();
#else
#error TODO: implement myself
#endif
}
