add_library(syntaxer syntax.cpp)
target_link_libraries(syntaxer PUBLIC lexer common_features)
target_include_directories(syntaxer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if (CompileTests)
    #add_subdirectory(tests)
endif()
