cmake_minimum_required(VERSION 3.22)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
cmake_policy(SET CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0092 NEW)
cmake_policy(SET CMP0092 NEW)

project(Firework.Runtime)

# Configuration options...
option(FIREWORK_WINDOWS_COMPATIBILITY "Enable compatibility with Windows versions up to Windows XP. This option only has effect when compiling with MinGW." ON)
option(FIREWORK_NO_CONSOLE "Build as a standalone program." ON)
option(FIREWORK_BUILD_EXAMPLES "Build various examples." ON)
option(FIREWORK_LOCAL_DBGINFO_PATHS "Use local paths embedded in program for debug information." OFF)
option(FIREWORK_SANITIZE "Enable supported sanitizers." OFF)
option(FIREWORK_LTO "Enable link-time optimization on release builds." OFF)

include(GenerateExportHeader)

set(FIREWORK_COMPILE_DEFINITIONS "")
set(FIREWORK_COMPILE_FLAGS "")
set(FIREWORK_LINKER_FLAGS "")

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(FIREWORK_TARGET_ARCH x64)
else()
    set(FIREWORK_TARGET_ARCH x86)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(FIREWORK_COMPILE_DEFINITIONS ${FIREWORK_COMPILE_DEFINITIONS} _DEBUG=1)
endif()

if (EMSCRIPTEN)
    add_compile_options(-pthread -fexceptions -O3)
    add_link_options(-pthread -fexceptions -sASYNCIFY -sPTHREAD_POOL_SIZE=8 -sPTHREAD_POOL_SIZE_STRICT=8 -sALLOW_MEMORY_GROWTH -sMIN_WEBGL_VERSION=2) # -sMIN_WEBGL_VERSION=2 is required for bgfx.
    set(FIREWORK_BUILD_PLATFORM Web)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
elseif (WIN32 OR WIN64)
    set(FIREWORK_BUILD_PLATFORM "Win32")
elseif (UNIX AND NOT APPLE)
    set(FIREWORK_BUILD_PLATFORM "Linux")
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
elseif (APPLE)
    set(FIREWORK_BUILD_PLATFORM "MacOS")
else()
    set(FIREWORK_BUILD_PLATFORM "UnknownPlatform")
endif()

if (NOT "${CMAKE_CXX_SIMULATE_ID} " STREQUAL " ")
    set(FIREWORK_COMPILER_ID "${CMAKE_CXX_SIMULATE_ID}")
else()
    set(FIREWORK_COMPILER_ID "${CMAKE_CXX_COMPILER_ID}")
endif()
if (EMSCRIPTEN)
    set(FIREWORK_COMPILER "Clang")
    set(FIREWORK_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
    set(FIREWORK_TARGET_NAME "${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-${FIREWORK_BUILD_TYPE}-${FIREWORK_BUILD_PLATFORM}")
elseif (FIREWORK_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_SHARED_LIBRARY_SUFFIX STREQUAL ".dll")
        set(FIREWORK_COMPILER "MinGW")
    else()
        set(FIREWORK_COMPILER "gcc")
    endif()
    set(FIREWORK_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
    set(FIREWORK_TARGET_NAME "${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-${FIREWORK_BUILD_TYPE}-${FIREWORK_BUILD_PLATFORM}")
    # Your copy of gcc better include a. Exceptions, and b. dynamic linking to libstdc++ and libgcc, or you're (silently) screwed.
    set(FIREWORK_COMPILE_DEFINITIONS ${FIREWORK_COMPILE_DEFINITIONS})
    set(FIREWORK_COMPILE_FLAGS ${FIREWORK_COMPILE_FLAGS} -fnon-call-exceptions -fasynchronous-unwind-tables)
    if (FIREWORK_BUILD_PLATFORM STREQUAL "Win32")
        set(FIREWORK_COMPILE_FLAGS ${FIREWORK_COMPILE_FLAGS} -fno-omit-frame-pointer) # Do NOT delete this, you will ruin my week if you do. Optimizing out the frame pointer prevents SEH exceptions from working properly.
    endif()
    if (FIREWORK_LOCAL_DBGINFO_PATHS)
        set(FIREWORK_COMPILE_FLAGS ${FIREWORK_COMPILE_FLAGS} -ffile-prefix-map="${CMAKE_SOURCE_DIR}/"=/)
    endif()
    if (FIREWORK_BUILD_PLATFORM STREQUAL "Win32" AND FIREWORK_WINDOWS_COMPATIBILITY)
        set(FIREWORK_COMPILE_DEFINITIONS ${FIREWORK_COMPILE_DEFINITIONS} _WIN32_WINNT=0x0501 WINVER=0x0501 __REQUIRED_RPCNDR_H_VERSION__=475)
    endif()

    if (FIREWORK_SANITIZE)
        set(FIREWORK_COMPILE_FLAGS ${FIREWORK_COMPILE_FLAGS} -fsanitize=undefined)
        set(FIREWORK_LINKER_FLAGS ${FIREWORK_LINKER_FLAGS} -fsanitize=undefined)
        if (FIREWORK_BUILD_PLATFORM STREQUAL "Win32")
            set(FIREWORK_COMPILE_FLAGS ${FIREWORK_COMPILE_FLAGS} -fsanitize-undefined-trap-on-error)
            set(FIREWORK_LINKER_FLAGS ${FIREWORK_LINKER_FLAGS} -fsanitize-undefined-trap-on-error)
            message(STATUS "[FireworkConfigure] [INFO]  MinGW sanitizers: -fsanitize=undefined -fsanitize-undefined-trap-on-error")
        else()
            set(FIREWORK_COMPILE_FLAGS ${FIREWORK_COMPILE_FLAGS} -fsanitize=leak)
            set(FIREWORK_LINKER_FLAGS ${FIREWORK_LINKER_FLAGS} -fsanitize=leak)
            message(STATUS "[FireworkConfigure] [INFO]  gcc sanitizers: -fsanitize=undefined -fsanitize=leak")
        endif()
    endif()
elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU" AND (FIREWORK_COMPILER_ID STREQUAL "Clang" OR FIREWORK_COMPILER_ID STREQUAL "AppleClang"))
    set(FIREWORK_COMPILER "Clang")
    set(FIREWORK_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
    set(FIREWORK_TARGET_NAME "${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-${FIREWORK_BUILD_TYPE}-${FIREWORK_BUILD_PLATFORM}")
    set(FIREWORK_COMPILE_FLAGS ${FIREWORK_COMPILE_FLAGS} -fms-extensions -fsafe-buffer-usage-suggestions) # TODO: -fasync-exceptions once everyone updates their version of Clang.
    set(FIREWORK_LINKER_FLAGS ${FIREWORK_LINKER_FLAGS} -pthread)
    
    if (FIREWORK_SANITIZE)
        set(FIREWORK_LINKER_FLAGS ${FIREWORK_LINKER_FLAGS} -fsanitize=undefined)
        if (FIREWORK_BUILD_PLATFORM STREQUAL "Win32")
            set(FIREWORK_LINKER_FLAGS ${FIREWORK_LINKER_FLAGS} -fsanitize=address)
            message(STATUS "[FireworkConfigure] [INFO]  clang sanitizers: -fsanitize=undefined -fsanitize=address")
        else()
            set(FIREWORK_LINKER_FLAGS ${FIREWORK_LINKER_FLAGS} -fsanitize=leak)
            message(STATUS "[FireworkConfigure] [INFO]  clang sanitizers: -fsanitize=undefined -fsanitize=leak")
        endif()
    endif()
elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC" OR FIREWORK_COMPILER_ID STREQUAL "MSVC")
    set(FIREWORK_COMPILER "MSVC")
    set(FIREWORK_BUILD_TYPE "Any")
    set(FIREWORK_TARGET_NAME "${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-${FIREWORK_BUILD_PLATFORM}")
    string(REPLACE " /EHs" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REPLACE " /EHs" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(FIREWORK_COMPILE_FLAGS ${FIREWORK_COMPILE_FLAGS} /Zc:__cplusplus /utf-8 /EHa)
    list(APPEND CMAKE_CXX_FLAGS /utf-8) # Otherwise assimp adds conflicting /source-charset:utf-8.

    if (FIREWORK_SANITIZE)
        set(FIREWORK_COMPILE_FLAGS ${FIREWORK_COMPILE_FLAGS} /fsanitize=address)
        message(STATUS "[FireworkConfigure] [INFO]  MSVC sanitizers: /fsanitize=address")
    endif()
else()
    set(FIREWORK_COMPILER ${FIREWORK_COMPILER_ID})
    if ("${FIREWORK_COMPILER} " STREQUAL " ")
        set(FIREWORK_COMPILER "Unknown")
    endif()
    set(FIREWORK_BUILD_TYPE "UnknownBuildType")
    set(FIREWORK_TARGET_NAME "${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-${FIREWORK_BUILD_PLATFORM}")
endif()

message(STATUS "FIREWORK_COMPILE_DEFINITIONS: ${FIREWORK_COMPILE_DEFINITIONS}")
message(STATUS "FIREWORK_COMPILE_FLAGS: ${FIREWORK_COMPILE_FLAGS}")
message(STATUS "FIREWORK_LINKER_FLAGS: ${FIREWORK_LINKER_FLAGS}")
add_compile_definitions(${FIREWORK_COMPILE_DEFINITIONS})
add_compile_options(${FIREWORK_COMPILE_FLAGS})
add_link_options(${FIREWORK_LINKER_FLAGS})

set(CMAKE_CXX_SCAN_FOR_MODULES OFF) # Don't break our build workflow.
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ${FIREWORK_LTO})
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL ${FIREWORK_LTO})
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ${FIREWORK_LTO})

# concurrentqueue
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/dependencies/concurrentqueue")

# robin-hood-hashing
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/dependencies/robin-hood-hashing")

# pugixml
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/pugixml")

# cpptrace
if (FIREWORK_BUILD_PLATFORM STREQUAL "Win32" OR FIREWORK_BUILD_PLATFORM STREQUAL "MacOS" OR FIREWORK_BUILD_PLATFORM STREQUAL "Linux")
    set(CPPTRACE_STATIC ON CACHE INTERNAL "" FORCE)
    set(CPPTRACE_PROVIDE_EXPORT_SET_FOR_LIBDWARF OFF CACHE INTERNAL "" FORCE)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/cpptrace")
    if (FIREWORK_COMPILER STREQUAL "MSVC")
        target_compile_definitions(cpptrace-lib PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
    set_target_properties(cpptrace-lib PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# SDL3
set(SDL_STATIC_PIC ON CACHE INTERNAL "" FORCE)
set(SDL_STATIC ON CACHE INTERNAL "" FORCE)
set(SDL_SHARED OFF CACHE INTERNAL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE INTERNAL "" FORCE)
set(SDL_RENDER_D3D OFF CACHE INTERNAL "" FORCE)
set(SDL_RENDER_METAL OFF CACHE INTERNAL "" FORCE)
set(HAVE_SHELLSCALINGAPI_H OFF CACHE INTERNAL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/SDL")

# glm
set(GLM_ENABLE_CXX_20 ON CACHE INTERNAL "" FORCE)
set(GLM_ENABLE_LANG_EXTENSIONS ON CACHE INTERNAL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/dependencies/glm")
target_compile_definitions(glm PUBLIC GLM_FORCE_LEFT_HANDED)

# assimp
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE INTERNAL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE INTERNAL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/assimp")
get_target_property(ASSIMP_LINK_LIBRARIES assimp LINK_LIBRARIES)
# external/assimp/code/CMakeLists.txt - 1348: TARGET_LINK_LIBRARIES ( assimp -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lwinpthread ) -> Breaks our binary.
# This doesn't actually work. Thanks assimp for ruining my day.
list(REMOVE_ITEM ASSIMP_LINK_LIBRARIES "-static-libgcc" "-static-libstdc++" "-Wl,-Bstatic" "-lstdc++" "-lwinpthread")
set_target_properties(assimp PROPERTIES LINK_LIBRARIES "${ASSIMP_LINK_LIBRARIES}")
unset(BUILD_SHARED_LIBS)

# bgfx

set(BGFX_LIBRARY_TYPE "STATIC" CACHE INTERNAL "" FORCE)
set(BGFX_BUILD_EXAMPLES ON CACHE INTERNAL "" FORCE)
set(BGFX_CONFIG_MULTITHREADED OFF CACHE INTERNAL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake")

# libcxxext
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/libcxxext")

# Firework.Runtime

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_DEBUG_POSTFIX "" CACHE INTERNAL "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
if (FIREWORK_COMPILER STREQUAL "MinGW" OR FIREWORK_COMPILER STREQUAL "gcc")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
elseif (FIREWORK_COMPILER STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmost -Wno-c++98-compat -Wno-c++0x-compat -Wno-c++11-compat -Wno-c++14-compat -Wno-c++17-compat -Wno-c++20-compat")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmost -Wno-c++98-compat -Wno-c++0x-compat -Wno-c++11-compat -Wno-c++14-compat -Wno-c++17-compat -Wno-c++20-compat")
elseif (FIREWORK_COMPILER STREQUAL "MSVC")
    set(FIREWORK_COMPILE_FLAGS ${FIREWORK_COMPILE_FLAGS} /Zc:preprocessor /FI "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.CoreLib/src/__msvc_bgfx_fix.h")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

message(STATUS "[FireworkConfigure] [INFO]  Compilation target ${FIREWORK_TARGET_NAME}.")

# Firework.Tools
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Firework.Tools")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/package_archive.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/precompile_shaders.cmake")

# Firework.Typography

file(GLOB_RECURSE FONTSRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Typography/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Typography/*.cpp"
)

add_library(Firework.Typography SHARED ${FONTSRCS})

generate_export_header(Firework.Typography
    BASE_NAME FIREWORK_TYPOGRAPHY
    EXPORT_MACRO_NAME _fw_typ_api
    EXPORT_FILE_NAME Firework.Typography.Exports.h
)

target_include_directories(Firework.Typography
    PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Typography/src"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/earcut.hpp/include"
    PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/stb"
)

target_compile_definitions(Firework.Typography PUBLIC ${FIREWORK_COMPILE_DEFINITIONS})
target_compile_options(Firework.Typography PUBLIC ${FIREWORK_COMPILE_FLAGS})
target_link_options(Firework.Typography PUBLIC ${FIREWORK_LINKER_FLAGS})
set_target_properties(Firework.Typography
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Typography"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Typography"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Typography"
)

# Firework.Runtime.GL

file(GLOB_RECURSE GLSRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.GL/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.GL/*.cpp"
)
target_compile_shaders(shaders_Firework.Runtime.GL "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.GL/shaders")

add_library(Firework.Runtime.GL SHARED ${GLSRCS})
add_dependencies(Firework.Runtime.GL shaders_Firework.Runtime.GL)

generate_export_header(Firework.Runtime.GL
    BASE_NAME FIREWORK_GL
    EXPORT_MACRO_NAME _fw_gl_api
    EXPORT_FILE_NAME Firework.Runtime.GL.Exports.h
)

target_include_directories(Firework.Runtime.GL
    PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.GL/src"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.GL/include"
    "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include"
    PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.GL/shaders/include"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies"
    "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bgfx/include"
    "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bimg/include"
    "${CMAKE_CURRENT_LIST_DIR}/external/assimp/include"
    "${CMAKE_CURRENT_BINARY_DIR}/external/assimp/include"
)
target_link_libraries(Firework.Runtime.GL
    PUBLIC
    assimp
    glm
    robin_hood
    sys
    PRIVATE
    bimg
    bx
    bgfx
)
if (FIREWORK_BUILD_PLATFORM STREQUAL "Win32")
    if (FIREWORK_COMPILER STREQUAL "MinGW" OR FIREWORK_COMPILER STREQUAL "Clang")
        target_include_directories(Firework.Runtime.GL PUBLIC "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include/compat/mingw")
    elseif (FIREWORK_COMPILER STREQUAL "MSVC")
        target_include_directories(Firework.Runtime.GL PUBLIC "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include/compat/msvc")
    endif()
elseif (FIREWORK_BUILD_PLATFORM STREQUAL "Linux")
    target_include_directories(Firework.Runtime.GL PUBLIC "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include/compat/linux")
elseif (FIREWORK_BUILD_PLATFORM STREQUAL "MacOS")
    target_include_directories(Firework.Runtime.GL PUBLIC "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include/compat/osx")
endif()
if (FIREWORK_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Firework.Runtime.GL PUBLIC BX_CONFIG_DEBUG=1)
else()
    target_compile_definitions(Firework.Runtime.GL PUBLIC BX_CONFIG_DEBUG=0)
endif()

target_compile_definitions(Firework.Runtime.GL PUBLIC ${FIREWORK_COMPILE_DEFINITIONS})
target_compile_options(Firework.Runtime.GL PUBLIC ${FIREWORK_COMPILE_FLAGS})
target_link_options(Firework.Runtime.GL PUBLIC ${FIREWORK_LINKER_FLAGS})
set_target_properties(Firework.Runtime.GL
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Runtime.GL"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Runtime.GL"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Runtime.GL"
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
)

# Firework.Runtime.RenderPipeline

file(GLOB_RECURSE RPSRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.RenderPipeline/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.RenderPipeline/*.cpp"
)

add_library(Firework.Runtime.RenderPipeline SHARED ${RPSRCS})

generate_export_header(Firework.Runtime.RenderPipeline
    BASE_NAME FIREWORK_RP
    EXPORT_MACRO_NAME _fw_rp_api
    EXPORT_FILE_NAME Firework.Runtime.RenderPipeline.Exports.h
)

target_include_directories(Firework.Runtime.RenderPipeline
    PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.RenderPipeline/src"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.RenderPipeline/shaders/include"
    "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bgfx/include"
)
target_link_libraries(Firework.Runtime.RenderPipeline
    PUBLIC
    Firework.Runtime.GL
    sys
)

target_compile_definitions(Firework.Runtime.RenderPipeline PUBLIC ${FIREWORK_COMPILE_DEFINITIONS})
target_compile_options(Firework.Runtime.RenderPipeline PUBLIC ${FIREWORK_COMPILE_FLAGS})
target_link_options(Firework.Runtime.RenderPipeline PUBLIC ${FIREWORK_LINKER_FLAGS})
set_target_properties(Firework.Runtime.RenderPipeline
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Runtime.RenderPipeline"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Runtime.RenderPipeline"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Runtime.RenderPipeline"
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
)

# Firework.Runtime.CoreLib

file(GLOB_RECURSE ENGINESRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.CoreLib/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.CoreLib/*.inc"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.CoreLib/*.cpp"
)

add_library(Firework.Runtime.CoreLib SHARED ${ENGINESRCS})

generate_export_header(Firework.Runtime.CoreLib
    BASE_NAME FIREWORK_CORELIB
    EXPORT_MACRO_NAME _fw_core_api
    EXPORT_FILE_NAME Firework.Runtime.CoreLib.Exports.h
)

target_include_directories(Firework.Runtime.CoreLib
    PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.CoreLib/src"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.CoreLib/include"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.CoreLib/module"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/std_function"
    "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bgfx/include"
    "${CMAKE_CURRENT_LIST_DIR}/external/SDL/include"
    PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include"
)
if (FIREWORK_COMPILER STREQUAL "MinGW" OR FIREWORK_COMPILER STREQUAL "Clang")
    target_include_directories(Firework.Runtime.CoreLib PRIVATE "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include/compat/mingw")
    target_link_libraries(Firework.Runtime.CoreLib PUBLIC stdc++exp)
elseif (FIREWORK_COMPILER STREQUAL "MSVC")
    target_include_directories(Firework.Runtime.CoreLib PRIVATE "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include/compat/msvc")
endif()
target_link_libraries(Firework.Runtime.CoreLib
    PUBLIC
    concurrentqueue
    Firework.Runtime.RenderPipeline
    Firework.Runtime.GL
    glm
    robin_hood
    sys
    PRIVATE
    SDL3-static
)
if (FIREWORK_NO_CONSOLE AND FIREWORK_BUILD_PLATFORM STREQUAL "Win32")
    if (FIREWORK_COMPILER STREQUAL "MinGW" OR FIREWORK_COMPILER STREQUAL "Clang")
        target_link_options(Firework.Runtime.CoreLib PUBLIC -mwindows)
    elseif (FIREWORK_COMPILER STREQUAL "MSVC")
        target_link_options(Firework.Runtime.CoreLib PUBLIC /SUBSYSTEM:WINDOWS)
    endif()
endif()
if (FIREWORK_BUILD_PLATFORM STREQUAL "Win32" OR FIREWORK_BUILD_PLATFORM STREQUAL "MacOS" OR FIREWORK_BUILD_PLATFORM STREQUAL "Linux")
    target_link_libraries(Firework.Runtime.CoreLib PRIVATE cpptrace-lib)
    if (FIREWORK_COMPILER STREQUAL "MinGW" OR FIREWORK_COMPILER STREQUAL "gcc")
        target_compile_options(cpptrace-lib PRIVATE -include "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.CoreLib/src/__mingw_cpptrace_fix.h")
        # When using libdwarf, avoid dynamic zlib dependency.
        # This zlib comes from some mystery subdirectory.
        target_link_libraries(Firework.Runtime.CoreLib PRIVATE zlibstatic)
    endif()
endif()
if (FIREWORK_BUILD_PLATFORM STREQUAL "Win32")
    target_link_libraries(Firework.Runtime.CoreLib PRIVATE psapi)
endif()

target_compile_definitions(Firework.Runtime.CoreLib PUBLIC ${FIREWORK_COMPILE_DEFINITIONS})
target_compile_options(Firework.Runtime.CoreLib PUBLIC ${FIREWORK_COMPILE_FLAGS})
target_link_options(Firework.Runtime.CoreLib PUBLIC ${FIREWORK_LINKER_FLAGS})
set_target_properties(Firework.Runtime.CoreLib
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Runtime.CoreLib"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Runtime.CoreLib"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Runtime.CoreLib"
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
)

# Firework.Components.Core2D

file(GLOB_RECURSE CORE2DSRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Components.Core2D/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Components.Core2D/*.cpp"
)
target_compile_shaders(shaders_Firework.Components.Core2D "${CMAKE_CURRENT_LIST_DIR}/Firework.Components.Core2D/shaders")

add_library(Firework.Components.Core2D SHARED ${CORE2DSRCS})
add_dependencies(Firework.Components.Core2D shaders_Firework.Components.Core2D)

generate_export_header(Firework.Components.Core2D
    BASE_NAME FIREWORK_COMPONENTCORE2D
    EXPORT_MACRO_NAME _fw_cc2d_api
    EXPORT_FILE_NAME Firework.Components.Core2D.Exports.h
)

target_include_directories(Firework.Components.Core2D
    PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Typography/src"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.GL/src"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Components.Core2D/src"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/stb"
    "${CMAKE_CURRENT_LIST_DIR}/external/cpptrace/include"
    PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Components.Core2D/shaders/include"
)
target_link_libraries(Firework.Components.Core2D
    PUBLIC
    glm
    pugixml-static
    sys
    Firework.Runtime.CoreLib
    Firework.Runtime.GL
    Firework.Typography
)

target_compile_definitions(Firework.Components.Core2D PUBLIC ${FIREWORK_COMPILE_DEFINITIONS})
target_compile_options(Firework.Components.Core2D PUBLIC ${FIREWORK_COMPILE_FLAGS})
target_link_options(Firework.Components.Core2D PUBLIC ${FIREWORK_LINKER_FLAGS})
set_target_properties(Firework.Components.Core2D
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Components.Core2D"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Components.Core2D"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Components.Core2D"
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
)

# Examples and tests.

file(GLOB FIREWORK_EXAMPLES LIST_DIRECTORIES TRUE RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/examples" "${CMAKE_CURRENT_SOURCE_DIR}/examples/*")

if (FIREWORK_BUILD_EXAMPLES)
    foreach (EXAMPLE_NAME ${FIREWORK_EXAMPLES})
        if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/examples/${EXAMPLE_NAME}/")
            continue()
        endif()

        if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/examples/${EXAMPLE_NAME}/assets")
            target_package_archive(package_${EXAMPLE_NAME} "${CMAKE_CURRENT_LIST_DIR}/examples/${EXAMPLE_NAME}/assets" "$<TARGET_FILE_DIR:example_${EXAMPLE_NAME}>/Runtime/CorePackage.fwpkg")
        endif()

        add_executable(example_${EXAMPLE_NAME} "${CMAKE_CURRENT_LIST_DIR}/examples/${EXAMPLE_NAME}/main.cpp")
        add_dependencies(example_${EXAMPLE_NAME} package_${EXAMPLE_NAME})

        target_link_libraries(example_${EXAMPLE_NAME} PUBLIC Firework.Components.Core2D)
        set_target_properties(example_${EXAMPLE_NAME}
            PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/example_${EXAMPLE_NAME}"
            LIBRARY_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/example_${EXAMPLE_NAME}"
            RUNTIME_OUTPUT_DIRECTORY "../bin/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/example_${EXAMPLE_NAME}"
            C_VISIBILITY_PRESET hidden
            CXX_VISIBILITY_PRESET hidden
        )

        add_custom_command(TARGET example_${EXAMPLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Firework.Typography>
            $<TARGET_FILE_DIR:example_${EXAMPLE_NAME}>
        )
        add_custom_command(TARGET example_${EXAMPLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Firework.Runtime.GL>
            $<TARGET_FILE_DIR:example_${EXAMPLE_NAME}>
        )
        add_custom_command(TARGET example_${EXAMPLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Firework.Runtime.RenderPipeline>
            $<TARGET_FILE_DIR:example_${EXAMPLE_NAME}>
        )
        add_custom_command(TARGET example_${EXAMPLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Firework.Runtime.CoreLib>
            $<TARGET_FILE_DIR:example_${EXAMPLE_NAME}>
        )
        add_custom_command(TARGET example_${EXAMPLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Firework.Components.Core2D>
            $<TARGET_FILE_DIR:example_${EXAMPLE_NAME}>
        )
    endforeach()
endif()
