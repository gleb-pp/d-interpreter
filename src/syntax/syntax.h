#pragma once

#include <optional>
#include <vector>
#include <memory>

class SyntaxAnalyzer {
public:
    static std::optional<std::shared_ptr<AST::Program>> analyze(
        const std::vector<std::shared_ptr<Token>>& tokens,
        const std::shared_ptr<const locators::CodeFile>& file,
        complog::ICompilationLog& log);
};