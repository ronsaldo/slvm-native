# SLVM native runtime cmake build script.
cmake_minimum_required(VERSION 3.0)
project(SlvmNativeRuntime)

# Check the build type
if (CMAKE_BUILD_TYPE STREQUAL "")
	# CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up
	# differentiation between debug and release builds.
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64")
	option(BUILD_I386_VERSION "Build x86 32 bits version" OFF)
	if(BUILD_I386_VERSION)
		if(WIN32)
	        set(CMAKE_ASM_NASM_OBJECT_FORMAT win32)
	    elseif(APPLE)
	        set(CMAKE_ASM_NASM_OBJECT_FORMAT macho)
	    else()
	    	set(CMAKE_ASM_NASM_OBJECT_FORMAT elf)
	    endif()
		set(CMAKE_ASM-ATT_FLAGS "--32")
		set(CMAKE_CXX_FLAGS "-m32")
		set(CMAKE_C_FLAGS "-m32")
	endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Output paths.
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dist")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/;${CMAKE_MODULE_PATH}")

# Turn warnings and use C++ 2011.
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# using Visual Studio C++
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -pthread")

	# Export symbols from applications.
	#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--export-dynamic")
endif()

# Perform platform checks
include(${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
include(${CMAKE_ROOT}/Modules/CheckIncludeFileCXX.cmake)
include(${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
include(${CMAKE_ROOT}/Modules/CheckLibraryExists.cmake)

# Enable the assembler.
if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86" OR "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64")
	if(WIN32)
		set(CMAKE_ASM_NASM_FLAGS -D_WIN32)
	elseif(APPLE)
		set(CMAKE_ASM_NASM_FLAGS -D_APPLE)
	else()
		set(CMAKE_ASM_NASM_FLAGS -D_UNIX)
	endif()

	enable_language(ASM_NASM)
endif()

# Set output dir.
set(EXECUTABLE_OUTPUT_PATH "${SlvmNativeRuntime_BINARY_DIR}/dist")
set(LIBRARY_OUTPUT_PATH "${SlvmNativeRuntime_BINARY_DIR}/dist")

# Add the current directory.
include_directories(
.
"${PROJECT_SOURCE_DIR}/include"
)

# Build the runtime
add_subdirectory(src)
