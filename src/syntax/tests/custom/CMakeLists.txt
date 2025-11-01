set(files "empty-var.d;newlines.d")
foreach (file IN LISTS files)
    add_custom_command(OUTPUT ${file}
        COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/${file} ${file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file} VERBATIM)
endforeach()
add_custom_target(syntaxer_files_custom DEPENDS ${files})
add_dependencies(syntaxer syntaxer_files_custom)
