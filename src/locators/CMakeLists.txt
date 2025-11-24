add_library(locators STATIC CodeFile.cpp locator.cpp)
target_link_libraries(locators PRIVATE common_features)
target_include_directories(locators PUBLIC include)

target_sources(dinterptools PRIVATE $<TARGET_OBJECTS:locators>)
target_include_directories(dinterptools PUBLIC include)
if (CompileTests)
    add_subdirectory(tests)
endif()
