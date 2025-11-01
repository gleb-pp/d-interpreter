#include <gtest/gtest.h>

#include <memory>

#include "complog/CompilationLog.h"
#include "lexer.h"
using namespace std;

static const char* CODE =
    R"%%(12345 19999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999
    193041934932402394293492034902903409230923953402934568938455.9)%%";

TEST(goodtests, biginttest) {
    shared_ptr<locators::CodeFile> file = make_shared<locators::CodeFile>("Test1.d", CODE);
    complog::AccumulatedCompilationLog log;
    auto maybeTokens = Lexer::tokenize(file, log);
    ASSERT_TRUE(maybeTokens.has_value());
    ASSERT_EQ(log.Messages().size(), 0ul);
    auto& tokens = maybeTokens.value();
    pair<Token::Type, Span> tokenInfo[] = {
        {Token::Type::tkIntLiteral, {0, 5}},
        {Token::Type::tkIntLiteral, {6, 101}},
        {Token::Type::tkNewLine, {107, 1}},
        {Token::Type::tkRealLiteral, {112, 62}},
        {Token::Type::tkEof, {string(CODE).size(), 0}},
    };

    constexpr size_t COUNT = sizeof(tokenInfo) / sizeof(tokenInfo[0]);
    ASSERT_EQ(tokens.size(), COUNT);

    for (size_t i = 0; i < COUNT; i++) {
        auto& tk = *tokens[i];
        EXPECT_EQ(tk.type, tokenInfo[i].first);
        EXPECT_EQ(tk.span, tokenInfo[i].second);
    }
    EXPECT_EQ(dynamic_pointer_cast<IntegerToken>(tokens[0])->value, 12345);
    BigInt pow10 = 10000;  // 100 = 0b1100100 = 4 + 32 + 64
    BigInt googol = 10000;
    pow10 *= pow10;  // 8
    pow10 *= pow10;  // 16
    pow10 *= pow10;  // 32
    googol *= pow10;
    pow10 *= pow10;  // 64
    googol *= pow10;
    EXPECT_EQ(dynamic_pointer_cast<IntegerToken>(tokens[1])->value, googol * BigInt(2) - BigInt(1));
    EXPECT_NEAR(dynamic_pointer_cast<RealToken>(tokens[3])->value,
                193041934932402394293492034902903409230923953402934568938455.9, 10);
}
