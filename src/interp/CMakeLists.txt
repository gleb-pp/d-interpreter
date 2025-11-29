add_library(interp closure.cpp execution.cpp input.cpp runner.cpp runtimeContext.cpp unaryOpExec.cpp userCallable.cpp
            variable.cpp varScopes.cpp)
target_link_libraries(interp PRIVATE common_features)
target_link_libraries(interp PUBLIC semantics)
target_include_directories(interp PUBLIC include)

target_sources(dinterptools PRIVATE $<TARGET_OBJECTS:interp>)
target_include_directories(dinterptools INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_sources(dinterptools PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
    include/dinterp/interp.h
    include/dinterp/interp/variable.h
    include/dinterp/interp/runner.h
    include/dinterp/interp/unaryOpExec.h
    include/dinterp/interp/execution.h
    include/dinterp/interp/varScopes.h
    include/dinterp/interp/input.h
    include/dinterp/interp/userCallable.h
    include/dinterp/interp/closure.h
    include/dinterp/interp/runtimeContext.h
)

if (Testing)
    add_subdirectory(tests)
endif()
