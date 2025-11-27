set(files "")
foreach (i RANGE 1 15)
    list(APPEND files ${i})
endforeach()
list(TRANSFORM files PREPEND 0 FOR 0 8)
list(TRANSFORM files APPEND .d)
foreach (file IN LISTS files)
    add_custom_command(OUTPUT ${file}
        COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/${file} ${file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file} VERBATIM)
endforeach()
add_custom_target(interp_files_basic DEPENDS ${files})
add_dependencies(interp_tests interp_files_basic)
