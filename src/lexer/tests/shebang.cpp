#include <gtest/gtest.h>

#include <memory>

#include "dinterp/complog/CompilationLog.h"
#include "dinterp/lexer.h"
using namespace std;
using namespace dinterp;

static const char* CODE =
    R"%%(#!/bin/env dinterp
print "Hello\n"
)%%";

TEST(goodtests, shebang) {
    shared_ptr<locators::CodeFile> file = make_shared<locators::CodeFile>("Test1.d", CODE);
    complog::AccumulatedCompilationLog log;
    auto maybeTokens = Lexer::tokenize(file, log, true);
    ASSERT_TRUE(maybeTokens.has_value());
    ASSERT_EQ(log.Messages().size(), 0ul);
    auto& tokens = maybeTokens.value();
    constexpr size_t COUNT = 4;
    ASSERT_EQ(tokens.size(), COUNT);
    pair<Token::Type, Span> tokenInfo[] = {
        {Token::Type::tkPrint, {file->Position(1, 0), 5}},          // print
        {Token::Type::tkStringLiteral, {file->Position(1, 6), 9}},  // "Hello\n"
        {Token::Type::tkNewLine, {file->Position(1, 15), 1}},       // \n

        {Token::Type::tkEof, {string(CODE).size(), 0}},
    };
    for (size_t i = 0; i < COUNT; i++) {
        auto& tk = *tokens[i];
        EXPECT_EQ(tk.type, tokenInfo[i].first);
        EXPECT_EQ(tk.span, tokenInfo[i].second);
    }
    auto pString = dynamic_pointer_cast<StringLiteral>(tokens[1]);
    ASSERT_NE(pString, nullptr);
    ASSERT_EQ(pString->value, "Hello\n");
}
