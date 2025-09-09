#pragma once
#include "locators/locator.h"
#include "locators/CodeFile.h"
#include "complog/CompilationLog.h"

#include <vector>
#include <memory>
#include <string>
#include <utility>
#include <optional>

struct Span {
public:
    size_t position;
    size_t length;
};

class LexerError : public complog::CompilationMessage {
private:
    locators::Locator position;
public:
    LexerError(const locators::Locator& position);
    void WriteMessageToStream(std::ostream& out, const FormatOptions& opts) const override;
    std::vector<locators::Locator> Locators() const override;
    virtual ~LexerError() override = default;
};

class NewlineInStringLiteralError : public complog::CompilationMessage {
private:
    locators::Locator position;
public:
    NewlineInStringLiteralError(const locators::Locator& position);
    void WriteMessageToStream(std::ostream& out, const FormatOptions& opts) const override;
    std::vector<locators::Locator> Locators() const override;
    virtual ~NewlineInStringLiteralError() override = default;
};

class WrongEscapeSequenceError : public complog::CompilationMessage {
private:
    locators::Locator position;
    std::string badsequence;
public:
    WrongEscapeSequenceError(const locators::Locator& position, const std::string& badsequence);
    void WriteMessageToStream(std::ostream& out, const FormatOptions& opts) const override;
    std::vector<locators::Locator> Locators() const override;
    virtual ~WrongEscapeSequenceError() override = default;
};

class Token {
public: 
    Span span;
    enum class Type {
        tkGreater,
        tkGreaterEq,
        tkLess,
        tkLessEq,
        tkEqual,
        tkNotEqual,
        tkPlus,
        tkMinus,
        tkTimes,
        tkDivide,
        tkNewLine,
        tkAssign,
        tkOpenBracket,
        tkClosedBracket,
        tkOpenParenthesis,
        tkClosedParenthesis,
        tkOpenCurlyBrace,
        tkClosedCurlyBrace,
        tkSemicolon,
        tkVar,
        tkWhile,
        tkFor,
        tkIf,
        tkThen,
        tkEnd,
        tkArrow,
        tkExit,
        tkPrint,
        tkRange,
        tkIn,
        tkElse,
        tkLoop,
        tkDot,
        tkComma,
        tkAnd,
        tkOr,
        tkNot,
        tkXor,
        tkInt,
        tkReal,
        tkBool,
        tkString,
        tkNone,
        tkFunc,
        tkIntLiteral,
        tkRealLiteral,
        tkStringLiteral,
        tkIdent
    } type;

    static const std::vector<std::pair<std::string, Type>> typeChars;
    virtual ~Token() = default;
};


class IdentifierToken : public Token {
public:
    std::string identifier;
    virtual ~IdentifierToken() override = default;
};

class IntegerToken : public Token {
public:
    long value;
    virtual ~IntegerToken() override = default;
};

class RealToken : public Token {
public:
    long double value;
    virtual ~RealToken() override = default;
};

class StringLiteral : public Token {
public:
    std::string value;
    virtual ~StringLiteral() override = default;
};

class Lexer {
public:
    static std::optional<std::vector<std::shared_ptr<Token>>> tokenize(
        const std::shared_ptr<const locators::CodeFile>& file, complog::ICompilationLog& log);
};
