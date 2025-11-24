add_library(interp closure.cpp execution.cpp input.cpp runner.cpp runtimeContext.cpp unaryOpExec.cpp userCallable.cpp
            variable.cpp varScopes.cpp)
target_link_libraries(interp PRIVATE common_features)
target_link_libraries(interp PUBLIC semantics)
target_include_directories(interp PUBLIC include)

target_sources(dinterptools PRIVATE $<TARGET_OBJECTS:interp>)
target_include_directories(dinterptools PUBLIC include)
if (CompileTests)
    add_subdirectory(tests)
endif()
