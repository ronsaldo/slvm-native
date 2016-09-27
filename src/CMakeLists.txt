set(Spur_Sources
    spur/objectmodel.c
)

set(Classes_Sources
    classes/collections.c
    classes/kernel.c
    classes/magnitude.c
)

set(All_Sources
    ${Spur_Sources}
    ${Classes_Sources}
)

add_library(SlvmNativeRuntime STATIC ${All_Sources})
