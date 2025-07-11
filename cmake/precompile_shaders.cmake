function(target_compile_shaders CL_TARGET_NAME SHADER_FILES_DIR)
    add_custom_target(${CL_TARGET_NAME} ALL
        ${CMAKE_COMMAND}
        -DSHADERC_PATH="$<TARGET_FILE:shaderc>"
        -DSHADERC_DIR="$<TARGET_FILE_DIR:shaderc>"
        -DSHADER_FILES_DIR="${SHADER_FILES_DIR}"
        -DBGFX_SHADER_DIR="${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bgfx/src"
        -DCMAKE_BUILD_TYPE=$<CONFIG>
        -P "${CMAKE_SOURCE_DIR}/cmake/precompile_shaders_script.cmake"
    )
    add_dependencies(${CL_TARGET_NAME} shaderc)
endfunction()
