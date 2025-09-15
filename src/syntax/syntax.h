#pragma once

#include <optional>
#include <vector>
#include <memory>
#include "complog/CompilationLog.h"
#include "complog/CompilationMessage.h"
#include "lexer.h"

class SyntaxErrorReport {
public:
    size_t rightmostPos = 0;
    std::vector<std::shared_ptr<complog::CompilationMessage>> messages;
    void Report(size_t pos, const std::shared_ptr<complog::CompilationMessage>& msg);
};

namespace ast {
class Statement;

class Program {
public:
    std::vector<std::shared_ptr<Statement>> statements;
    static std::optional<std::shared_ptr<Program>> Parse(
        const std::vector<std::shared_ptr<Token>>& tokens,
        const std::shared_ptr<const locators::CodeFile>& file,
        complog::ICompilationLog& log);
};
}

class SyntaxAnalyzer {
public:
    static std::optional<std::shared_ptr<ast::Program>> analyze(
        const std::vector<std::shared_ptr<Token>>& tokens,
        const std::shared_ptr<const locators::CodeFile>& file,
        complog::ICompilationLog& log);
};
