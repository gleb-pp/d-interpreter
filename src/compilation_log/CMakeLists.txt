add_library(compilationlog STATIC CompilationLog.cpp CompilationMessage.cpp)
target_link_libraries(compilationlog PRIVATE common_features)
target_link_libraries(compilationlog PUBLIC locators)
target_include_directories(compilationlog PUBLIC include)

if (CompileTests)
    add_subdirectory(tests)
endif()
