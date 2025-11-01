#pragma once
#include <complog/CompilationMessage.h>
#include <iostream>
#include "lexer.h"

class EmptyVarStatement : public complog::CompilationMessage {
private:
    locators::Locator loc;

public:
    EmptyVarStatement(locators::Locator position);
    void WriteMessageToStream(std::ostream& out, [[maybe_unused]] const FormatOptions& options) const override;
    std::vector<locators::Locator> Locators() const override;
    virtual ~EmptyVarStatement() override = default;
};

class UnexpectedTokenTypeError : public complog::CompilationMessage {
private:
    locators::Locator loc;
    std::vector<Token::Type> expected;
    Token::Type found;

public:
    UnexpectedTokenTypeError(locators::Locator position, const std::vector<Token::Type>& expected, Token::Type found);
    void WriteMessageToStream(std::ostream& out, [[maybe_unused]] const FormatOptions& options) const override;
    std::vector<locators::Locator> Locators() const override;
    virtual ~UnexpectedTokenTypeError() override = default;
};
