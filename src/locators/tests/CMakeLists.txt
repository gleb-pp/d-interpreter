add_executable(locators_test CodeFileTests.cpp fixture.cpp LocatorTests.cpp main.cpp)
target_link_libraries(locators_test PRIVATE test_features locators)
add_test(NAME locators_test COMMAND locators_test)
