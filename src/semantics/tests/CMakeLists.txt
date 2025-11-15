add_executable(semantics_test fixture.cpp main.cpp)
target_link_libraries(semantics_test PRIVATE semantics test_features)
add_test(NAME semantics_test COMMAND semantics_test)

add_subdirectory(demos)
