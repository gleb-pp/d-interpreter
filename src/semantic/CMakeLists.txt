add_library(semantics astDeepCopy.cpp diagnostics.cpp expressionChecker.cpp precomputed.cpp semantic.cpp
            statementChecker.cpp unaryOpsChecker.cpp valueTimeline.cpp)
target_link_libraries(semantics PUBLIC syntaxer runtime)
target_link_libraries(semantics PRIVATE common_features)
target_include_directories(semantics PUBLIC include)

target_sources(dinterptools PRIVATE $<TARGET_OBJECTS:semantics>)
target_include_directories(dinterptools INTERFACE $<BUILD_INTERFACE:include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_sources(dinterptools PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
    include/dinterp/semantic.h
    include/dinterp/semantic/expressionChecker.h
    include/dinterp/semantic/valueTimeline.h
    include/dinterp/semantic/statementChecker.h
    include/dinterp/semantic/diagnostics.h
    include/dinterp/semantic/unaryOpsChecker.h
    include/dinterp/syntaxext/precomputed.h
    include/dinterp/syntaxext/astDeepCopy.h
)

if (CompileTests)
    add_subdirectory(tests)
endif ()
