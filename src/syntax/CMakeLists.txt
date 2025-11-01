add_library(syntaxer syntax.cpp exceptions.cpp syntaxcontext.cpp tokenscanner.cpp)
target_link_libraries(syntaxer PUBLIC lexer common_features)
target_include_directories(syntaxer PUBLIC include)

if (CompileTests)
    add_subdirectory(tests)
endif()
