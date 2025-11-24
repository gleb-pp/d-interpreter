#include <gtest/gtest.h>

#include <memory>

#include "dinterp/complog/CompilationLog.h"
#include "dinterp/lexer.h"
using namespace std;

static const char* CODE = "<<===>>=></ /=/=...//=\n";
//                                            ^ this is a comment!

TEST(goodtests, test2) {
    shared_ptr<locators::CodeFile> file = make_shared<locators::CodeFile>("Test1.d", CODE);
    complog::AccumulatedCompilationLog log;
    auto maybeTokens = Lexer::tokenize(file, log);
    ASSERT_TRUE(maybeTokens.has_value());
    ASSERT_EQ(log.Messages().size(), 0ul);
    auto& tokens = maybeTokens.value();
    pair<Token::Type, Span> tokenInfo[] = {
        {Token::Type::tkLess, {0, 1}},       // <
        {Token::Type::tkLessEq, {1, 2}},     // <=
        {Token::Type::tkEqual, {3, 1}},      // =
        {Token::Type::tkArrow, {4, 2}},      // =>
        {Token::Type::tkGreaterEq, {6, 2}},  // >=
        {Token::Type::tkGreater, {8, 1}},    // >
        {Token::Type::tkLess, {9, 1}},       // <
        {Token::Type::tkDivide, {10, 1}},    // /
        {Token::Type::tkNotEqual, {12, 2}},  // /=
        {Token::Type::tkNotEqual, {14, 2}},  // /=
        {Token::Type::tkRange, {16, 2}},     // ..
        {Token::Type::tkDot, {18, 1}},       // .
        {Token::Type::tkNewLine, {22, 1}},   // \n
        {Token::Type::tkEof, {string(CODE).size(), 0}},
    };

    constexpr size_t COUNT = sizeof(tokenInfo) / sizeof(tokenInfo[0]);
    ASSERT_EQ(tokens.size(), COUNT);

    for (size_t i = 0; i < COUNT; i++) {
        auto& tk = *tokens[i];
        EXPECT_EQ(tk.type, tokenInfo[i].first);
        EXPECT_EQ(tk.span, tokenInfo[i].second);
    }
}
