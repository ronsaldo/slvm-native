set(Spur_Sources
    spur/objectmodel.c
)

set(Classes_Sources
    classes/collections.c
    classes/context.c
    classes/ffi.c
    classes/kernel.c
    classes/magnitude.c
)

set(All_Sources
    lifecycle.c
    ${Spur_Sources}
    ${Classes_Sources}
)

add_library(SlvmNativeRuntime STATIC ${All_Sources})
