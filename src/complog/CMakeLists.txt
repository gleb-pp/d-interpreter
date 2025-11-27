add_library(complog STATIC CompilationLog.cpp CompilationMessage.cpp)
target_link_libraries(complog PRIVATE common_features)
target_link_libraries(complog PUBLIC locators)
target_include_directories(complog PUBLIC include)

target_sources(dinterptools PRIVATE $<TARGET_OBJECTS:complog>)
target_include_directories(dinterptools INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_sources(dinterptools PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
    include/dinterp/complog/CompilationMessage.h
    include/dinterp/complog/CompilationLog.h
)

if (Testing)
    add_subdirectory(tests)
endif()
