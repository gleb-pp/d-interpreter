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
    enum class Type {
        tkVar,
        tkNewLine,
        tkSemicolon,
        tkInt,
        tkFloat,
        tkString,
        tkWhile,
        tkFor,
        tkEqual,
        tkNotEqual
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
    static std::vector<std::shared_ptr<Token>> tokenize();
};
