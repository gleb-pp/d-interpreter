add_library(semantic astDeepCopy.cpp diagnostics.cpp expressionChecker.cpp precomputed.cpp semantic.cpp
            statementChecker.cpp unaryOpsChecker.cpp valueTimeline.cpp)
target_link_libraries(semantic PUBLIC syntax runtime_objects)
target_link_libraries(semantic PRIVATE common_features)
target_include_directories(semantic PUBLIC include)
