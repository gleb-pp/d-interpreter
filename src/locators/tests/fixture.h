#pragma once
#include <gtest/gtest.h>

#include "dinterp/locators/locator.h"

class RealCodeFixture : public testing::Test {
protected:
    static constexpr const char* CODE = R"%%%(#include <iostream>
using namespace std;

int main() {
    cout << "Hello, world!\n";
})%%%";
    std::shared_ptr<locators::CodeFile> file;
    virtual void SetUp() override;
    virtual void TearDown() override;
};
