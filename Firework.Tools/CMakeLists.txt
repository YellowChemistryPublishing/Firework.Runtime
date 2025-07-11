cmake_minimum_required(VERSION 3.10)

project(Firework.Tools)

add_executable(tool_Firework.Packager
    "${CMAKE_CURRENT_LIST_DIR}/src/packager_main.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/Packager.cpp"
)
target_include_directories(tool_Firework.Packager PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")
target_link_libraries(tool_Firework.Packager PRIVATE sys)

set_target_properties(tool_Firework.Packager
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/tool_Firework.Packager"
    LIBRARY_OUTPUT_DIRECTORY "../../lib/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/tool_Firework.Packager"
    RUNTIME_OUTPUT_DIRECTORY "../../bin/${FIREWORK_TARGET_ARCH}-${FIREWORK_COMPILER}-$<CONFIG>-${FIREWORK_BUILD_PLATFORM}/tool_Firework.Packager"
)
