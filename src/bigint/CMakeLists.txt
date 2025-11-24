add_library(bigint bigint.cpp)
target_link_libraries(bigint PRIVATE common_features)
target_include_directories(bigint PUBLIC include)

target_sources(dinterptools PRIVATE $<TARGET_OBJECTS:bigint>)
target_include_directories(dinterptools PUBLIC include)
if (CompileTests)
    add_subdirectory(tests)
endif()
