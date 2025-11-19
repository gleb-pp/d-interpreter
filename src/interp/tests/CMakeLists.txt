add_executable(interp_tests fixture.cpp main.cpp)
target_link_libraries(interp_tests PRIVATE test_features interp)
add_subdirectory(samples)
add_test(NAME interp_tests COMMAND interp_tests)
