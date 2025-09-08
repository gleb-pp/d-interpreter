#pragma once

#include <vector>
#include <memory>
#include <string>

struct Span {
public:
    size_t pos;
    size_t length;
};

class Token {
public: 
    Span span;
    static constexpr const char* typeStrs[] = {
        "var",
        "\n",
        ";",
        "while",
        "for",
        "if",
        "then",
        "end",
        "=>",
        "exit",
        ":=",
        "print",
        "..",
        "in",
        "else",
        "loop",
        "[",
        "]",
        "(",
        ")",
        ".",
        ",",
        "and",
        "or",
        "xor",
        "not",
        ">",
        ">=",
        "<",
        "<=",
        "=",
        "/=",
        "+",
        "-",
        "*",
        "/",
        "int",
        "real",
        "bool",
        "string",
        "none",
        "func",
        "{",
        "}",
    };
    enum class Type {
        tkVar,
        tkNewLine,
        tkSemicolon,
        tkWhile,
        tkFor,
        tkIf,
        tkThen,
        tkEnd,
        tkArrow,
        tkExit,
        tkAssign,
        tkPrint,
        tkRange,
        tkIn,
        tkElse,
        tkLoop,
        tkOpenBracket,
        tkClosedBracket,
        tkOpenParenthesis,
        tkClosedParenthesis,
        tkDot,
        tkComma,
        tkAnd,
        tkOr,
        tkXor,
        tkNot,
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
        tkInt,
        tkReal,
        tkBool,
        tkString,
        tkNone,
        tkFunc,
        tkOpenCurlyBrace,
        tkClosedCurlyBrace,
        tkIntLiteral,
        tkRealLiteral,
        tkStringLiteral,
        tkIdent,
    } type;
    Token(Span span, Type tkType);
};

class IdentifierToken : public Token {
public:
    std::string identifier;
    IdentifierToken(Span span, const std::string& value);
};

class IntegerToken : public Token {
public:
    long value;
    IntegerToken(Span span, long value);
};

class RealToken : public Token {
public:
    long double value;
    RealToken(Span span, long double value);
};

class StringLiteral : public Token {
public:
    std::string value;
    StringLiteral(Span span, const std::string& value);
};

class Lexer {
public:
    static std::vector<std::shared_ptr<Token>> tokenize(const std::string& code);
};
