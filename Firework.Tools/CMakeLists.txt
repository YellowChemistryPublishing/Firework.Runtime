cmake_minimum_required(VERSION 3.10)

project(Firework.Tools)

add_executable(Firework.Packager
    "${CMAKE_CURRENT_LIST_DIR}/src/packager_main.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/Packager.cpp"
)
target_include_directories(Firework.Packager PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")

set_target_properties(Firework.Packager
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Packager"
    LIBRARY_OUTPUT_DIRECTORY "../../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Packager"
    RUNTIME_OUTPUT_DIRECTORY "../../bin/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.Packager"
)

add_executable(Firework.ShaderCompiler
    "${CMAKE_CURRENT_LIST_DIR}/src/packager.sc_main.cpp"
)
target_include_directories(Firework.ShaderCompiler PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")
target_compile_definitions(Firework.ShaderCompiler PRIVATE CMAKE_EXECUTABLE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX})

set_target_properties(Firework.ShaderCompiler
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.ShaderCompiler"
    LIBRARY_OUTPUT_DIRECTORY "../../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.ShaderCompiler"
    RUNTIME_OUTPUT_DIRECTORY "../../bin/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/Firework.ShaderCompiler"
)
add_custom_command(TARGET Firework.ShaderCompiler POST_BUILD
    COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:shaderc>
    $<TARGET_FILE_DIR:Firework.ShaderCompiler>
)
