#include <gtest/gtest.h>
#include <memory>
#include "complog/CompilationLog.h"
#include "lexer.h"
using namespace std;

static const char* CODE =
R"%%(var i := 5.5
                ;;;
for if  _somename90311 007othername
"A string lite\\\\ral that does not support escape sequences """
goodbye end
)%%";

TEST(goodtests, test1) {
    shared_ptr<locators::CodeFile> file = make_shared<locators::CodeFile>("Test1.d", CODE);
    complog::AccumulatedCompilationLog log;
    auto maybeTokens = Lexer::tokenize(file, log);
    ASSERT_TRUE(maybeTokens.has_value());
    ASSERT_EQ(log.Messages().size(), 0ul);
    auto& tokens = maybeTokens.value();
    constexpr size_t COUNT = 5 + 4 + 6 + 3 + 3 + 1;
    ASSERT_EQ(tokens.size(), COUNT);
    pair<Token::Type, Span> tokenInfo[] = {
        {Token::Type::tkVar,         {file->Position(0, 0), 3}},  // var
        {Token::Type::tkIdent,       {file->Position(0, 4), 1}},  // i
        {Token::Type::tkAssign,      {file->Position(0, 6), 2}},  // :=
        {Token::Type::tkRealLiteral, {file->Position(0, 9), 3}},  // 5.5
        {Token::Type::tkNewLine,     {file->Position(0, 12), 1}}, // \n

        {Token::Type::tkSemicolon,   {file->Position(1, 16), 1}}, // ;
        {Token::Type::tkSemicolon,   {file->Position(1, 17), 1}}, // ;
        {Token::Type::tkSemicolon,   {file->Position(1, 18), 1}}, // ;
        {Token::Type::tkNewLine,     {file->Position(1, 19), 1}}, // \n

        {Token::Type::tkFor,         {file->Position(2, 0), 3}},  // for
        {Token::Type::tkIf,          {file->Position(2, 4), 2}},  // if
        {Token::Type::tkIdent,       {file->Position(2, 8), 14}}, // _somename90311
        {Token::Type::tkIntLiteral,  {file->Position(2, 23), 3}}, // 007
        {Token::Type::tkIdent,       {file->Position(2, 26), 9}}, // othername
        {Token::Type::tkNewLine,     {file->Position(2, 35), 1}}, // \n

        {Token::Type::tkStringLiteral, {file->Position(3, 0), 62}}, // the first string
        {Token::Type::tkStringLiteral, {file->Position(3, 62), 2}}, // the empty string
        {Token::Type::tkNewLine,     {file->Position(3, 64), 1}}, // \n
        
        {Token::Type::tkIdent,       {file->Position(4, 0), 7}},  // goodbye
        {Token::Type::tkEnd,         {file->Position(4, 8), 3}},  // end
        {Token::Type::tkNewLine,     {file->Position(4, 11), 1}}, // \n

        {Token::Type::tkEof,         {string(CODE).size(), 0}},
    };
    for (size_t i = 0; i < COUNT; i++) {
        auto& tk = *tokens[i];
        EXPECT_EQ(tk.type, tokenInfo[i].first);
        EXPECT_EQ(tk.span, tokenInfo[i].second);
    }
    auto pIdent = dynamic_pointer_cast<IdentifierToken>(tokens[1]);
    ASSERT_NE(pIdent, nullptr);
    ASSERT_EQ(pIdent->identifier, "i");
    auto pReal = dynamic_pointer_cast<RealToken>(tokens[3]);
    ASSERT_NE(pReal, nullptr);
    ASSERT_EQ(pReal->value, 5.5);
    pIdent = dynamic_pointer_cast<IdentifierToken>(tokens[11]);
    ASSERT_NE(pIdent, nullptr);
    ASSERT_EQ(pIdent->identifier, "_somename90311");
    auto pInt = dynamic_pointer_cast<IntegerToken>(tokens[12]);
    ASSERT_NE(pInt, nullptr);
    ASSERT_EQ(pInt->value, 7ul);
    pIdent = dynamic_pointer_cast<IdentifierToken>(tokens[13]);
    ASSERT_NE(pIdent, nullptr);
    ASSERT_EQ(pIdent->identifier, "othername");
    auto pString = dynamic_pointer_cast<StringLiteral>(tokens[15]);
    ASSERT_NE(pString, nullptr);
    ASSERT_EQ(pString->value, "A string lite\\\\ral that does not support escape sequences ");
    pString = dynamic_pointer_cast<StringLiteral>(tokens[16]);
    ASSERT_NE(pString, nullptr);
    ASSERT_EQ(pString->value, "");
    pIdent = dynamic_pointer_cast<IdentifierToken>(tokens[18]);
    ASSERT_NE(pIdent, nullptr);
    ASSERT_EQ(pIdent->identifier, "goodbye");
}
