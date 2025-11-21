set(files "")
foreach (i RANGE 1 39)
    list(APPEND files ${i})
endforeach()
list(TRANSFORM files PREPEND 0 FOR 0 8)
list(TRANSFORM files APPEND .d)
foreach (file IN LISTS files)
    add_custom_command(OUTPUT ${file}
        COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/${file} ${file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file} VERBATIM)
endforeach()
add_custom_target(semantics_demos DEPENDS ${files})
add_dependencies(semantics semantics_demos)

