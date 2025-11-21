add_executable(complog_test baderror.cpp badwarning.cpp fixture.cpp main.cpp LogTests.cpp
    MessageTests.cpp SeverityTests.cpp spanerror.cpp)
target_link_libraries(complog_test PRIVATE test_features complog)
add_test(NAME complog_test COMMAND complog_test)
