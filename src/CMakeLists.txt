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

if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86" OR "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64")
   set(Native_Sources
       native/x86/trampolines.s
       native/x86/send.c
   )
endif()

set(All_Sources
    lifecycle.c
    datastructures.c
    ${Spur_Sources}
    ${Classes_Sources}
    ${Native_Sources}
)

add_library(SlvmNativeRuntime STATIC ${All_Sources})
