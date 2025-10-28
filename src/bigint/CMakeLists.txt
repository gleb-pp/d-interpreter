add_library(bigint bigint.cpp)
target_link_libraries(bigint PRIVATE common_features)
target_include_directories(include)
