add_executable(lexer_tests main.cpp goodtest1.cpp goodtest2.cpp badtest1.cpp badtest2.cpp badtest3.cpp biginttest.cpp
    shebang.cpp)
target_link_libraries(lexer_tests PRIVATE test_features lexer)
add_test(NAME lexer_tests COMMAND lexer_tests)
