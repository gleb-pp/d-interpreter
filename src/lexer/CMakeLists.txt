add_library(lexer STATIC lexer.cpp)
target_include_directories(lexer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lexer PUBLIC common_features compilationlog locators bigint)

if (CompileTests)
    add_subdirectory(tests)
endif()
