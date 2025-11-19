#pragma once
#include <gtest/gtest.h>
#include "locators/CodeFile.h"
#include "complog/CompilationLog.h"
#include "syntax.h"

namespace interp {

class Sample : public testing::Test {
public:
    std::shared_ptr<const locators::CodeFile> file;
    const char* filename;
    complog::AccumulatedCompilationLog log;
    std::shared_ptr<ast::Body> program;
    void ReadFile(const char* name, bool compiles);
    void RunAndExpect(const char* input, const char* output);
    void RunAndExpectCrash(const char* input);
};

}
