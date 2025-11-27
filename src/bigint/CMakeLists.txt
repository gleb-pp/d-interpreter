add_library(bigint bigint.cpp)
target_link_libraries(bigint PRIVATE common_features)
target_include_directories(bigint PUBLIC include)

target_sources(dinterptools PRIVATE $<TARGET_OBJECTS:bigint>)
target_include_directories(dinterptools INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_sources(dinterptools PUBLIC FILE_SET HEADERS BASE_DIRS include FILES include/dinterp/bigint.h)

if (CompileTests)
    add_subdirectory(tests)
endif()
