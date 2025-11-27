add_library(runtime types.cpp values.cpp derror.cpp)
target_link_libraries(runtime PRIVATE common_features)
target_link_libraries(runtime PUBLIC syntaxer lexer complog locators)
target_include_directories(runtime PUBLIC include)

target_sources(dinterptools PRIVATE $<TARGET_OBJECTS:runtime>)
target_include_directories(dinterptools INTERFACE $<BUILD_INTERFACE:include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_sources(dinterptools PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
    include/dinterp/runtime/derror.h
    include/dinterp/runtime/types.h
    include/dinterp/runtime/values.h
    include/dinterp/runtime.h
)
