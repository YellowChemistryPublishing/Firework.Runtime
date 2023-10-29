include(${CMAKE_CURRENT_LIST_DIR}/cmake-bin2h/bin2h.cmake)

if (WIN32 OR WIN64)
    set(GL_PLATFORM Win32)
elseif (UNIX AND NOT APPLE)
    set(GL_PLATFORM Linux)
elseif (APPLE)
    set(GL_PLATFORM MacOS)
else()
    set(GL_PLATFORM "UnknownPlatform")
endif()

message(STATUS "[ShaderPrecompiler] [TRACE] Compiling shaders...")

file(MAKE_DIRECTORY "${SHADER_FILES_DIR}/build")

macro(ExecShaderCompiler SHADER_NAME SHADER_EXT PROFILE_ARG BACKEND_NAME)
    if (SHADER_EXT STREQUAL ".vert")
        set(SHADER_TYPE "Vertex")
    elseif (SHADER_EXT STREQUAL ".frag")
        set(SHADER_TYPE "Fragment")
    else()
        set(SHADER_TYPE "Compute")
    endif()
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        set(DEBUG "--debug")
    else()
        set(DEBUG "")
    endif()

    execute_process(
        COMMAND "${SHADERC_PATH}" -f "${SHADER_FILES_DIR}/${SHADER_NAME}${SHADER_EXT}.sc" -o "${SHADER_FILES_DIR}/build/${SHADER_NAME}${SHADER_EXT}.${BACKEND_NAME}.bin" --type "${SHADER_TYPE}"
        -p "${PROFILE_ARG}" --varyingdef "${SHADER_FILES_DIR}/varying.def.sc" -i "${BGFX_SHADER_DIR}" ${DEBUG}
        WORKING_DIRECTORY "${SHADERC_DIR}"
        RESULT_VARIABLE SHADERC_RETURN
    )

    if (SHADERC_RETURN AND NOT SHADERC_RETURN EQUAL 0)
        file(REMOVE "${SHADER_FILES_DIR}/build/${SHADER_FULL_NAME}.md5")
        message(FATAL_ERROR "[ShaderPrecompiler] [FATAL] Shader compilation of ${SHADER_TYPE} shader ${SHADER_NAME} for ${BACKEND_NAME} failed with code [${SHADERC_RETURN}].")
    else()
        message(STATUS "[ShaderPrecompiler] [INFO]  Shader compilation of ${SHADER_TYPE} shader ${SHADER_NAME} for ${BACKEND_NAME} succeeded.")
    endif()

    bin2h(
        SOURCE_FILE "${SHADER_FILES_DIR}/build/${SHADER_NAME}${SHADER_EXT}.${BACKEND_NAME}.bin"
        HEADER_FILE "${SHADER_FILES_DIR}/include/${SHADER_NAME}${SHADER_EXT}.${BACKEND_NAME}.h"
        VARIABLE_NAME "shader${SHADER_NAME}${SHADER_TYPE}${BACKEND_NAME}Data"
    )
endmacro()

macro(CreateShaderInclude SHADER_NAME SHADER_EXT)
    if (SHADER_EXT STREQUAL ".vert")
        if (GL_PLATFORM STREQUAL Win32)
            ExecShaderCompiler(${SHADER_NAME} ${SHADER_EXT} "s_3_0" "d3d9")
            ExecShaderCompiler(${SHADER_NAME} ${SHADER_EXT} "s_4_0" "d3d11")
            ExecShaderCompiler(${SHADER_NAME} ${SHADER_EXT} "s_5_0" "d3d12")
        endif()
        ExecShaderCompiler(${SHADER_NAME} ${SHADER_EXT} "120" "opengl")
        ExecShaderCompiler(${SHADER_NAME} ${SHADER_EXT} "spirv10-10" "vulkan")
    elseif (SHADER_EXT STREQUAL ".frag")
        if (GL_PLATFORM STREQUAL Win32)
            ExecShaderCompiler(${SHADER_NAME} ${SHADER_EXT} "s_3_0" "d3d9")
            ExecShaderCompiler(${SHADER_NAME} ${SHADER_EXT} "s_4_0" "d3d11")
            ExecShaderCompiler(${SHADER_NAME} ${SHADER_EXT} "s_5_0" "d3d12")
        endif()
        ExecShaderCompiler(${SHADER_NAME} ${SHADER_EXT} "120" "opengl")
        ExecShaderCompiler(${SHADER_NAME} ${SHADER_EXT} "spirv10-10" "vulkan")
    elseif (SHADER_EXT STREQUAL ".comp")
        if (GL_PLATFORM STREQUAL Win32)
            ExecShaderCompiler(${SHADER_NAME} ${SHADER_EXT} "s_3_0" "d3d9")
            ExecShaderCompiler(${SHADER_NAME} ${SHADER_EXT} "s_5_0" "d3d11")
            ExecShaderCompiler(${SHADER_NAME} ${SHADER_EXT} "s_5_0" "d3d12")
        endif()
        ExecShaderCompiler(${SHADER_NAME} ${SHADER_EXT} "430" "opengl")
        ExecShaderCompiler(${SHADER_NAME} ${SHADER_EXT} "spirv10-10" "vulkan")
    else()
        message(FATAL_ERROR "[ShaderPrecompiler] [FATAL] Unrecognised shader type of file ${SHADER}! Must be one of *.vert.sc, *.frag.sc, or *.comp.sc.")
    endif()
endmacro()

file(GLOB SHADER_SOURCE_FILES "${SHADER_FILES_DIR}/*.sc")

foreach (SHADER ${SHADER_SOURCE_FILES})
    get_filename_component(SHADER_FULL_NAME ${SHADER} NAME)
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    get_filename_component(SHADER_EXT ${SHADER} EXT)
    string(REPLACE ".sc" "" SHADER_EXT ${SHADER_EXT})

    file(MD5 ${SHADER} SHADER_HASH)
    if (EXISTS "${SHADER_FILES_DIR}/build/${SHADER_FULL_NAME}.md5")
        file(READ "${SHADER_FILES_DIR}/build/${SHADER_FULL_NAME}.md5" PREV_SHADER_HASH)
    else()
        set(PREV_SHADER_HASH "")
    endif()
    file(MD5 "${SHADER_FILES_DIR}/varying.def.sc" VARYING_HASH)
    if (EXISTS "${SHADER_FILES_DIR}/build/varying.def.sc.md5")
        file(READ "${SHADER_FILES_DIR}/build/varying.def.sc.md5" PREV_VARYING_HASH)
    else()
        set(PREV_VARYING_HASH "")
    endif()

    if (NOT SHADER_EXT STREQUAL ".def")
        if (NOT PREV_SHADER_HASH STREQUAL "${SHADER_HASH}" OR NOT PREV_VARYING_HASH STREQUAL "${VARYING_HASH}")
            CreateShaderInclude("${SHADER_NAME}" "${SHADER_EXT}")
            CreateShaderInclude("${SHADER_NAME}" "${SHADER_EXT}")
            file(WRITE "${SHADER_FILES_DIR}/build/${SHADER_FULL_NAME}.md5" "${SHADER_HASH}")
            file(WRITE "${SHADER_FILES_DIR}/build/varying.def.sc.md5" "${VARYING_HASH}")
            file(WRITE
                "${SHADER_FILES_DIR}/include/${SHADER_NAME}.vfAll.h"
"#pragma once\n\
\n\
#ifdef _WIN32\n\
#include <${SHADER_NAME}.vert.d3d9.h>\n\
#include <${SHADER_NAME}.frag.d3d9.h>\n\
#include <${SHADER_NAME}.vert.d3d11.h>\n\
#include <${SHADER_NAME}.frag.d3d11.h>\n\
#include <${SHADER_NAME}.vert.d3d12.h>\n\
#include <${SHADER_NAME}.frag.d3d12.h>\n\
#endif\n\
#include <${SHADER_NAME}.vert.opengl.h>\n\
#include <${SHADER_NAME}.frag.opengl.h>\n\
#include <${SHADER_NAME}.vert.vulkan.h>\n\
#include <${SHADER_NAME}.frag.vulkan.h>\n"
            )
        else()
            message(STATUS "[ShaderPrecompiler] [INFO]  Skipping compilation of shader ${SHADER}; source file unchanged.")
        endif()
    endif()
endforeach()

message(STATUS "[ShaderPrecompiler] [TRACE] Shader precompilation complete.")