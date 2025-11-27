set(files "array.d")
foreach (file IN LISTS files)
    add_custom_command(OUTPUT ${file}
        COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/${file} ${file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file} VERBATIM)
endforeach()
add_custom_target(interp_files_extra DEPENDS ${files})
add_dependencies(interp_tests interp_files_extra)
