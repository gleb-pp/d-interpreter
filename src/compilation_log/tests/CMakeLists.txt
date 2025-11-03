add_executable(compilationlog_test baderror.cpp badwarning.cpp fixture.cpp main.cpp LogTests.cpp
    MessageTests.cpp SeverityTests.cpp spanerror.cpp)
target_link_libraries(compilationlog_test PRIVATE test_features compilationlog)
add_test(NAME compilationlog_test COMMAND compilationlog_test)
