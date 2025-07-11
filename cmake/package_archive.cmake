function(target_package_archive PACKAGE_TARGET_NAME PACKAGE_INPUT_FOLDER PACKAGE_OUTPUT_FILE)
    add_custom_target(${PACKAGE_TARGET_NAME} ALL $<TARGET_FILE:tool_Firework.Packager> "${PACKAGE_INPUT_FOLDER}" --output "${PACKAGE_OUTPUT_FILE}")
    add_dependencies(${PACKAGE_TARGET_NAME} tool_Firework.Packager)
endfunction()
