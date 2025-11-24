add_library(semantics astDeepCopy.cpp diagnostics.cpp expressionChecker.cpp precomputed.cpp semantic.cpp
            statementChecker.cpp unaryOpsChecker.cpp valueTimeline.cpp)
target_link_libraries(semantics PUBLIC syntaxer runtime)
target_link_libraries(semantics PRIVATE common_features)
target_include_directories(semantics PUBLIC include)

if (CompileTests)
    add_subdirectory(tests)
endif ()
