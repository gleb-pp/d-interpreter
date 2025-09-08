#pragma once

#include <vector>
#include <memory>
#include <string>

struct Span {
public:
    size_t line;
    size_t position;
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
};

class Identifier : public Token {
public:
    std::string identifier;
};

class Integer : Token {
public:
    long value;
};

class Real : Token {
public:
    long double value;
};

class StringLiteral : Token {
public:
    std::string value;
};

class Lexer {
public:
    static std::vector<std::shared_ptr<Token>> tokenize(const std::string& code);
};
