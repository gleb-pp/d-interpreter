add_library(locators STATIC CodeFile.cpp locator.cpp)
target_link_libraries(locators PRIVATE common_features)
target_include_directories(locators PUBLIC include)

target_sources(dinterptools PRIVATE $<TARGET_OBJECTS:locators>)
target_include_directories(dinterptools INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_sources(dinterptools PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
    include/dinterp/locators/CodeFile.h
    include/dinterp/locators/locator.h
)

if (Testing)
    add_subdirectory(tests)
endif()
