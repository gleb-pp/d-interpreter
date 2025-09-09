add_executable(lexer_tests main.cpp )
target_link_libraries(lexer_tests PRIVATE test_features lexer)
add_test(NAME lexer_tests COMMAND lexer_tests)
