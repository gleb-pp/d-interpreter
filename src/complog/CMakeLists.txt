add_library(complog STATIC CompilationLog.cpp CompilationMessage.cpp)
target_link_libraries(complog PRIVATE common_features)
target_link_libraries(complog PUBLIC locators)
target_include_directories(complog PUBLIC include)

if (CompileTests)
    add_subdirectory(tests)
endif()
