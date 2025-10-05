add_executable(syntaxer_test fixture.cpp main.cpp)
target_link_libraries(syntaxer_test PRIVATE syntaxer test_features)
add_test(NAME syntaxer_test COMMAND syntaxer_test)

add_subdirectory(basic)
add_subdirectory(complex)
