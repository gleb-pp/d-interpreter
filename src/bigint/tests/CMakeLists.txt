add_executable(bigint_tests main.cpp arith_samples.cpp)
target_link_libraries(bigint_tests PRIVATE bigint pthread gtest test_features)
target_include_directories(bigint_tests PRIVATE ../include)

add_test(NAME bigint_tests COMMAND bigint_tests)
