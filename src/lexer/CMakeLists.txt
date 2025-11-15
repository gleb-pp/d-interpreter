add_library(lexer STATIC lexer.cpp)
target_include_directories(lexer PUBLIC include)
target_link_libraries(lexer PRIVATE common_features)
target_link_libraries(lexer PUBLIC complog locators bigint)

if (CompileTests)
    add_subdirectory(tests)
endif()
