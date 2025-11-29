#pragma once
#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "dinterp/complog/CompilationLog.h"
#include "dinterp/lexer.h"
#include "dinterp/locators/CodeFile.h"
#include "dinterp/syntax.h"

class FileSample : public testing::Test {
public:
    std::shared_ptr<const dinterp::locators::CodeFile> file;
    std::vector<std::shared_ptr<dinterp::Token>> tokens;
    std::unique_ptr<dinterp::complog::AccumulatedCompilationLog> log;
    std::shared_ptr<dinterp::ast::Body> program;
    void ReadFile(std::string name, bool expectSuccess);
    void ExpectFailure(size_t line, size_t col);
};
