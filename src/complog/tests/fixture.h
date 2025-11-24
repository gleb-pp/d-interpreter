#pragma once
#include <gtest/gtest.h>

#include <memory>

#include "dinterp/locators/CodeFile.h"
#include "dinterp/locators/locator.h"

class PascalProgram : public testing::Test {
protected:
    // line 5 (4 in 0-base): 28 chars
    static constexpr const char* CODE = R"%%%(program test;

var i: Integer;
begin
    for i := 100 downto 1 do
    begin
        writeln(i, "...");
    end;
    readln;
end.)%%%";
    std::shared_ptr<locators::CodeFile> file;
    virtual void SetUp() override;
    virtual void TearDown() override;
};
