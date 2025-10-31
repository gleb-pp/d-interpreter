add_library(bigint bigint.cpp)
target_link_libraries(bigint PRIVATE common_features)
target_include_directories(bigint PUBLIC include)

if (CompileTests)
    add_subdirectory(tests)
endif()
