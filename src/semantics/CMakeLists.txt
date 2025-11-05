add_library(semantics astDeepCopy.cpp diagnostics.cpp expressionChecker.cpp precomputed.cpp semantic.cpp
            statementChecker.cpp unaryOpsChecker.cpp valueTimeline.cpp)
target_link_libraries(semantics PUBLIC syntax runtime_objects)
target_link_libraries(semantics PRIVATE common_features)
target_include_directories(semantics PUBLIC include)
