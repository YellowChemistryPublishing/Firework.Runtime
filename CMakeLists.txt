cmake_minimum_required(VERSION 3.22)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
cmake_policy(SET CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0092 NEW)
cmake_policy(SET CMP0092 NEW)

project(Firework.Runtime)

# Configuration options...
option(FIREWORK_WINDOWS_COMPATIBILITY "Enable compatibility with Windows versions up to Windows XP. This option only has effect when compiling with MinGW." ON)
option(FIREWORK_BUILD_EXAMPLES "Build various examples." ON)
option(FIREWORK_SANITIZE "Enable supported sanitizers." OFF)
option(FIREWORK_LTO "Enable link-time optimization on release builds." OFF)

include(GenerateExportHeader)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(FIREWORK_TARGET_ARCH x64)
else()
    set(FIREWORK_TARGET_ARCH x86)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-D_DEBUG=1)
endif()

if (EMSCRIPTEN)
    add_compile_options(-pthread -fexceptions -O3)
    add_link_options(-pthread -fexceptions -sASYNCIFY -sPTHREAD_POOL_SIZE=8 -sPTHREAD_POOL_SIZE_STRICT=8 -sALLOW_MEMORY_GROWTH -sWASM=0)
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
    add_compile_options(-fnon-call-exceptions)
    if (FIREWORK_BUILD_PLATFORM STREQUAL "Win32" AND FIREWORK_WINDOWS_COMPATIBILITY)
        add_definitions(-D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D__REQUIRED_RPCNDR_H_VERSION__=475)
    endif()

    if (FIREWORK_SANITIZE)
        add_compile_options(-fsanitize=undefined)
        add_link_options(-fsanitize=undefined)
        if (FIREWORK_BUILD_PLATFORM STREQUAL "Win32")
            add_compile_options(-fsanitize-undefined-trap-on-error)
            add_link_options(-fsanitize-undefined-trap-on-error)
            message(STATUS "[FireworkConfigure] [INFO]  MinGW sanitizers: -fsanitize=undefined -fsanitize-undefined-trap-on-error")
        else()
            add_compile_options(-fsanitize=leak)
            add_link_options(-fsanitize=leak)
            message(STATUS "[FireworkConfigure] [INFO]  gcc sanitizers: -fsanitize=undefined -fsanitize=leak")
        endif()
    endif()
elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU" AND (FIREWORK_COMPILER_ID STREQUAL "Clang" OR FIREWORK_COMPILER_ID STREQUAL "AppleClang"))
    set(FIREWORK_COMPILER "Clang")
    set(FIREWORK_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
    set(FIREWORK_TARGET_NAME "${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-${FIREWORK_BUILD_TYPE}-${FIREWORK_BUILD_PLATFORM}")
    add_compile_options(-fms-extensions -fasynchronous-unwind-tables -fsafe-buffer-usage-suggestions) # -fasync-exceptions breaks clang. Let the bug crunching happen.
    add_link_options(-pthread)
    
    if (FIREWORK_SANITIZE)
        add_link_options(-fsanitize=undefined)
        if (FIREWORK_BUILD_PLATFORM STREQUAL "Win32")
            add_link_options(-fsanitize=address)
            message(STATUS "[FireworkConfigure] [INFO]  clang sanitizers: -fsanitize=undefined -fsanitize=address")
        else()
            add_link_options(-fsanitize=leak)
            message(STATUS "[FireworkConfigure] [INFO]  clang sanitizers: -fsanitize=undefined -fsanitize=leak")
        endif()
    endif()
elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC" OR FIREWORK_COMPILER_ID STREQUAL "MSVC")
    set(FIREWORK_COMPILER "MSVC")
    set(FIREWORK_BUILD_TYPE "Any")
    set(FIREWORK_TARGET_NAME "${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-${FIREWORK_BUILD_PLATFORM}")
    string(REPLACE " /EHs" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REPLACE " /EHs" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    add_compile_options(/Zc:__cplusplus /utf-8 /EHa)

    if (FIREWORK_SANITIZE)
        add_compile_options(/fsanitize=address)
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

macro(firework_push_nowarn)
    if (FIREWORK_COMPILER STREQUAL "MinGW" OR FIREWORK_COMPILER STREQUAL "gcc" OR FIREWORK_COMPILER STREQUAL "Clang")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
    elseif (FIREWORK_COMPILER STREQUAL "MSVC")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /w")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w")
    endif()
endmacro()
macro(firework_pop_nowarn)
    if (FIREWORK_COMPILER STREQUAL "MinGW" OR FIREWORK_COMPILER STREQUAL "gcc" OR FIREWORK_COMPILER STREQUAL "Clang")
        string(REPLACE " -w" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
        string(REPLACE " -w" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    elseif (FIREWORK_COMPILER STREQUAL "MSVC")
        string(REPLACE " /w" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
        string(REPLACE " /w" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif()
endmacro()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ${FIREWORK_LTO})
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL ${FIREWORK_LTO})
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ${FIREWORK_LTO})

firework_push_nowarn()

# concurrentqueue
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/dependencies/concurrentqueue")

# robin-hood-hashing
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/dependencies/robin-hood-hashing")

# uni-algo
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/uni-algo")

# cpptrace
firework_pop_nowarn()
if (CMAKE_SYSTEM_NAME STREQUAL Windows OR CMAKE_SYSTEM_NAME STREQUAL Darwin OR CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CPPTRACE_STATIC ON CACHE INTERNAL "" FORCE)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/cpptrace")
    if (FIREWORK_COMPILER STREQUAL "MSVC")
        target_compile_options(cpptrace PRIVATE /D_CRT_SECURE_NO_WARNINGS)
    endif()
    set_target_properties(cpptrace PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
firework_push_nowarn()

# SDL3
set(SDL_STATIC_PIC ON CACHE INTERNAL "" FORCE)
set(SDL_STATIC ON CACHE INTERNAL "" FORCE)
set(SDL_SHARED OFF CACHE INTERNAL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE INTERNAL "" FORCE)
set(SDL_RENDER_D3D OFF CACHE INTERNAL "" FORCE)
set(SDL_RENDER_METAL OFF CACHE INTERNAL "" FORCE)
set(HAVE_SHELLSCALINGAPI_H OFF CACHE INTERNAL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/SDL")
# target_compile_definitions(SDL3-static PRIVATE HAVE_LIBC=1) i don't know if we still need this

# assimp
firework_pop_nowarn()
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE INTERNAL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/assimp")
get_target_property(ASSIMP_LINK_LIBRARIES assimp LINK_LIBRARIES)
# external/assimp/code/CMakeLists.txt - 1348: TARGET_LINK_LIBRARIES ( assimp -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lwinpthread ) -> Breaks our binary.
list(REMOVE_ITEM ASSIMP_LINK_LIBRARIES "-static-libgcc" "-static-libstdc++" "-Wl,-Bstatic" "-lstdc++" "-lwinpthread")
set_target_properties(assimp PROPERTIES LINK_LIBRARIES "${ASSIMP_LINK_LIBRARIES}")
unset(BUILD_SHARED_LIBS)
firework_push_nowarn()

# bgfx

if (FIREWORK_COMPILER STREQUAL MinGW)
    add_definitions(-DMINGW_HAS_SECURE_API=0)
endif()

set(BGFX_LIBRARY_TYPE "STATIC" CACHE INTERNAL "" FORCE)
set(BGFX_BUILD_EXAMPLES ON CACHE INTERNAL "" FORCE)
set(BGFX_CONFIG_MULTITHREADED OFF CACHE INTERNAL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake")

if (FIREWORK_COMPILER STREQUAL "MinGW")
    remove_definitions(-DMINGW_HAS_SECURE_API)
endif()

# Firework.Runtime

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_DEBUG_POSTFIX "" CACHE INTERNAL "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
firework_pop_nowarn()
if (FIREWORK_COMPILER STREQUAL "MinGW" OR FIREWORK_COMPILER STREQUAL "gcc")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
elseif (FIREWORK_COMPILER STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmost -Wno-c++98-compat -Wno-c++0x-compat -Wno-c++11-compat -Wno-c++14-compat -Wno-c++17-compat -Wno-c++20-compat")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmost -Wno-c++98-compat -Wno-c++0x-compat -Wno-c++11-compat -Wno-c++14-compat -Wno-c++17-compat -Wno-c++20-compat")
elseif (FIREWORK_COMPILER STREQUAL "MSVC")
    add_compile_options(/Zc:preprocessor)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

message(STATUS "[FireworkConfigure] [INFO]  Compilation target ${FIREWORK_TARGET_NAME}.")

# Firework.Tools
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Firework.Tools")

# Firework.Mathematics
add_library(Firework.Mathematics INTERFACE)
target_include_directories(Firework.Mathematics INTERFACE "${CMAKE_CURRENT_LIST_DIR}/Firework.Mathematics/include")

# Firework.Typography

file(GLOB_RECURSE FONTSRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Typography/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Typography/*.cpp"
)

add_library(Firework.Typography SHARED ${FONTSRCS})

target_link_libraries(Firework.Typography PRIVATE uni-algo)

generate_export_header(Firework.Typography
    BASE_NAME FIREWORK_TYPOGRAPHY
    EXPORT_MACRO_NAME __firework_typography_api
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

set_target_properties(Firework.Typography
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Typography"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Typography"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Typography"
)

# Firework.Runtime.GL

add_custom_target(PrecompileBuiltinShaders
    COMMAND ${CMAKE_COMMAND}
    -DSHADERC_PATH="$<TARGET_FILE:shaderc>"
    -DSHADERC_DIR="$<TARGET_FILE_DIR:shaderc>"
    -DSHADER_FILES_DIR="${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.GL/shaders"
    -DBGFX_SHADER_DIR="${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bgfx/src"
    -DCMAKE_BUILD_TYPE=$<CONFIG>
    -P "${CMAKE_CURRENT_LIST_DIR}/cmake/PrecompileShaders.cmake"
)
add_dependencies(PrecompileBuiltinShaders shaderc)

file(GLOB_RECURSE GLSRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.GL/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.GL/*.cpp"
)

add_library(Firework.Runtime.GL SHARED ${GLSRCS})
add_dependencies(Firework.Runtime.GL PrecompileBuiltinShaders)

generate_export_header(Firework.Runtime.GL
    BASE_NAME FIREWORK_GL
    EXPORT_MACRO_NAME __firework_gl_api
    EXPORT_FILE_NAME Firework.Runtime.GL.Exports.h
)

target_include_directories(Firework.Runtime.GL
    PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.GL/src"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.GL/include"
    "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include"
    PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bgfx/include"
    "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bimg/include"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.GL/shaders/include"
    "${CMAKE_CURRENT_LIST_DIR}/external/assimp/include"
    "${CMAKE_CURRENT_BINARY_DIR}/external/assimp/include"
)
target_link_libraries(Firework.Runtime.GL
    PUBLIC
    Firework.Mathematics
    assimp
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

set_target_properties(Firework.Runtime.GL
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Runtime.GL"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Runtime.GL"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Runtime.GL"
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
)

# Firework.Runtime.RenderPipeline

add_custom_target(PrecompileRenderPipelineShaders
    COMMAND ${CMAKE_COMMAND}
    -DSHADERC_PATH="$<TARGET_FILE:shaderc>"
    -DSHADERC_DIR="$<TARGET_FILE_DIR:shaderc>"
    -DSHADER_FILES_DIR="${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.RenderPipeline/shaders"
    -DBGFX_SHADER_DIR="${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bgfx/src"
    -DCMAKE_BUILD_TYPE=$<CONFIG>
    -P "${CMAKE_CURRENT_LIST_DIR}/cmake/PrecompileShaders.cmake"
)
add_dependencies(PrecompileRenderPipelineShaders shaderc)

file(GLOB_RECURSE RPSRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.RenderPipeline/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.RenderPipeline/*.cpp"
)

add_library(Firework.Runtime.RenderPipeline SHARED ${RPSRCS})
add_dependencies(Firework.Runtime.GL PrecompileRenderPipelineShaders)

generate_export_header(Firework.Runtime.RenderPipeline
    BASE_NAME FIREWORK_RP
    EXPORT_MACRO_NAME __firework_rp_api
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
)

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
# target_sources(Firework.Runtime.CoreLib PUBLIC FILE_SET CXX_MODULES FILES "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.CoreLib/module/Firework.Core.cxx")

generate_export_header(Firework.Runtime.CoreLib
    BASE_NAME FIREWORK_CORELIB
    EXPORT_MACRO_NAME __firework_corelib_api
    EXPORT_FILE_NAME Firework.Runtime.CoreLib.Exports.h
)
# file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/Firework.Runtime.CoreLib.Exports.h" "\n#ifndef FIREWORK_EXPORT_MODULE\n    #define FIREWORK_EXPORT_MODULE 0\n#endif\n#ifndef __fw_module_export\n    #define __fw_module_export\n#endif\n")

target_compile_definitions(Firework.Runtime.CoreLib PUBLIC SDL_MAIN_HANDLED)
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
elseif (FIREWORK_COMPILER STREQUAL "MSVC")
    target_include_directories(Firework.Runtime.CoreLib PRIVATE "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include/compat/msvc")
endif()
target_link_libraries(Firework.Runtime.CoreLib
    PUBLIC
    concurrentqueue
    robin_hood
    Firework.Mathematics
    Firework.Runtime.RenderPipeline
    Firework.Runtime.GL
    PRIVATE
    SDL3-static
)
if (FIREWORK_BUILD_PLATFORM STREQUAL "Win32" OR FIREWORK_BUILD_PLATFORM STREQUAL "MacOS" OR FIREWORK_BUILD_PLATFORM STREQUAL "Linux")
    target_link_libraries(Firework.Runtime.CoreLib PRIVATE cpptrace)
    if (FIREWORK_COMPILER STREQUAL "MinGW" OR FIREWORK_COMPILER STREQUAL "gcc")
        # When using libdwarf, avoid dynamic zlib dependency.
        # This zlib comes from some mystery subdirectory.
        target_link_libraries(Firework.Runtime.CoreLib PRIVATE zlibstatic)
    endif()
endif()
if (FIREWORK_BUILD_PLATFORM STREQUAL "Win32")
    target_link_libraries(Firework.Runtime.CoreLib PRIVATE psapi)
endif()

set_target_properties(Firework.Runtime.CoreLib
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Runtime.CoreLib"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Runtime.CoreLib"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Runtime.CoreLib"
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
)

# Firework.Components.Core2D

add_custom_target(PrecompileComponentCore2DShaders
    COMMAND ${CMAKE_COMMAND}
    -DSHADERC_PATH="$<TARGET_FILE:shaderc>"
    -DSHADERC_DIR="$<TARGET_FILE_DIR:shaderc>"
    -DSHADER_FILES_DIR="${CMAKE_CURRENT_LIST_DIR}/Firework.Components.Core2D/shaders"
    -DBGFX_SHADER_DIR="${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bgfx/src"
    -DCMAKE_BUILD_TYPE=$<CONFIG>
    -P "${CMAKE_CURRENT_LIST_DIR}/cmake/PrecompileShaders.cmake"
)
add_dependencies(PrecompileComponentCore2DShaders shaderc)

file(GLOB_RECURSE GLSRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.GL/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.GL/*.cpp"
)

file(GLOB_RECURSE CORE2DSRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Components.Core2D/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Components.Core2D/*.cpp"
)

add_library(Firework.Components.Core2D SHARED ${CORE2DSRCS})
add_dependencies(Firework.Components.Core2D PrecompileComponentCore2DShaders)

generate_export_header(Firework.Components.Core2D
    BASE_NAME FIREWORK_COMPONENTCORE2D
    EXPORT_MACRO_NAME __firework_componentcore2d_api
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
    Firework.Mathematics
    Firework.Runtime.CoreLib
    Firework.Runtime.GL
    Firework.Typography
)

set_target_properties(Firework.Components.Core2D
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Components.Core2D"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Components.Core2D"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Components.Core2D"
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
)

# Firework.Components.Core3D

add_custom_target(PrecompileComponentCore3DShaders
    COMMAND ${CMAKE_COMMAND}
    -DSHADERC_PATH="$<TARGET_FILE:shaderc>"
    -DSHADERC_DIR="$<TARGET_FILE_DIR:shaderc>"
    -DSHADER_FILES_DIR="${CMAKE_CURRENT_LIST_DIR}/Firework.Components.Core3D/shaders"
    -DBGFX_SHADER_DIR="${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bgfx/src"
    -DCMAKE_BUILD_TYPE=$<CONFIG>
    -P "${CMAKE_CURRENT_LIST_DIR}/cmake/PrecompileShaders.cmake"
)
add_dependencies(PrecompileComponentCore3DShaders shaderc)

file(GLOB_RECURSE GLSRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.GL/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Runtime.GL/*.cpp"
)

file(GLOB_RECURSE CORE3DSRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Components.Core3D/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Components.Core3D/*.cpp"
)

add_library(Firework.Components.Core3D SHARED ${CORE3DSRCS})
add_dependencies(Firework.Components.Core3D PrecompileComponentCore3DShaders)

generate_export_header(Firework.Components.Core3D
    BASE_NAME FIREWORK_COMPONENTCORE3D
    EXPORT_MACRO_NAME __firework_componentcore3d_api
    EXPORT_FILE_NAME Firework.Components.Core3D.Exports.h
)

target_include_directories(Firework.Components.Core3D
    PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Components.Core3D/src"
    PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Firework.Components.Core3D/shaders/include"
)
target_link_libraries(Firework.Components.Core3D
    PUBLIC
    Firework.Runtime.CoreLib
)

set_target_properties(Firework.Components.Core3D
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Components.Core3D"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Components.Core3D"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Components.Core3D"
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
)

# Examples and tests.

list(APPEND EXAMPLES "HelloFireworkUI")
list(APPEND EXAMPLES "DevTest")
if (FIREWORK_BUILD_EXAMPLES)
    foreach (EXAMPLE_NAME ${EXAMPLES})
        string(REPLACE "." "_" EXAMPLE_NAME_NORMALIZED "${EXAMPLE_NAME}")
        add_custom_target(Package${EXAMPLE_NAME_NORMALIZED}Assets
            COMMAND ${CMAKE_COMMAND}
            -DFIREWORK_PACKAGER_EXECUTABLE="$<TARGET_FILE:Firework.Packager>"
            -DPACKAGE_INPUT_FOLDER="${CMAKE_CURRENT_LIST_DIR}/examples/${EXAMPLE_NAME}/Assets"
            -DPACKAGE_OUTPUT_FILE="$<TARGET_FILE_DIR:${EXAMPLE_NAME}>/Runtime/CorePackage.fwpkg"
            -P "${CMAKE_CURRENT_LIST_DIR}/cmake/PackageArchive.cmake"
        )
        add_dependencies(Package${EXAMPLE_NAME_NORMALIZED}Assets Firework.Packager)

        add_executable(${EXAMPLE_NAME} "${CMAKE_CURRENT_LIST_DIR}/examples/${EXAMPLE_NAME}/main.cpp")
        # target_sources(${EXAMPLE_NAME} PUBLIC FILE_SET CXX_MODULES FILES "${CMAKE_CURRENT_LIST_DIR}/examples/${EXAMPLE_NAME}/main.cpp")
        add_dependencies(${EXAMPLE_NAME} Package${EXAMPLE_NAME_NORMALIZED}Assets)

        target_link_libraries(${EXAMPLE_NAME} PUBLIC Firework.Components.Core2D Firework.Components.Core3D)
        set_target_properties(${EXAMPLE_NAME}
            PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Examples/${EXAMPLE_NAME}"
            LIBRARY_OUTPUT_DIRECTORY "../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Examples/${EXAMPLE_NAME}"
            RUNTIME_OUTPUT_DIRECTORY "../bin/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Examples/${EXAMPLE_NAME}"
            C_VISIBILITY_PRESET hidden
            CXX_VISIBILITY_PRESET hidden
        )

        add_custom_command(TARGET ${EXAMPLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Firework.Typography>
            $<TARGET_FILE_DIR:${EXAMPLE_NAME}>
        )
        add_custom_command(TARGET ${EXAMPLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Firework.Runtime.GL>
            $<TARGET_FILE_DIR:${EXAMPLE_NAME}>
        )
        add_custom_command(TARGET ${EXAMPLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Firework.Runtime.RenderPipeline>
            $<TARGET_FILE_DIR:${EXAMPLE_NAME}>
        )
        add_custom_command(TARGET ${EXAMPLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Firework.Runtime.CoreLib>
            $<TARGET_FILE_DIR:${EXAMPLE_NAME}>
        )
        add_custom_command(TARGET ${EXAMPLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Firework.Components.Core2D>
            $<TARGET_FILE_DIR:${EXAMPLE_NAME}>
        )
        add_custom_command(TARGET ${EXAMPLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Firework.Components.Core3D>
            $<TARGET_FILE_DIR:${EXAMPLE_NAME}>
        )
    endforeach()
endif()

if (FIREWORK_BUILD_PLATFORM STREQUAL "Win32")
    remove_definitions(-D_WIN32_WINNT)
endif()
