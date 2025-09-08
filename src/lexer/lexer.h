#pragma once

#include <vector>
#include <memory>
#include <string>
#include <utility>

struct Span {
public:
    size_t position;
    size_t length;
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
};

const std::vector<std::pair<std::string, Token::Type>> Token::typeChars = {
    std::make_pair("var", Token::Type::tkVar),
    std::make_pair("while", Token::Type::tkWhile),
    std::make_pair("for", Token::Type::tkFor),
    std::make_pair("if", Token::Type::tkIf),
    std::make_pair("then", Token::Type::tkThen),
    std::make_pair("end", Token::Type::tkEnd),
    std::make_pair("exit", Token::Type::tkExit),
    std::make_pair("print", Token::Type::tkPrint),
    std::make_pair("else", Token::Type::tkElse),
    std::make_pair("loop", Token::Type::tkLoop),
    std::make_pair(",", Token::Type::tkComma),
    std::make_pair("and", Token::Type::tkAnd),
    std::make_pair("or", Token::Type::tkOr),
    std::make_pair("not", Token::Type::tkNot),
    std::make_pair("xor", Token::Type::tkXor),
    std::make_pair("real", Token::Type::tkReal),
    std::make_pair("string", Token::Type::tkString),
    std::make_pair("bool", Token::Type::tkBool),
    std::make_pair("none", Token::Type::tkNone),
    std::make_pair("func", Token::Type::tkFunc),
    std::make_pair("+", Token::Type::tkPlus),
    std::make_pair("-", Token::Type::tkMinus),
    std::make_pair("*", Token::Type::tkTimes),
    std::make_pair("\n", Token::Type::tkNewLine),
    std::make_pair("[", Token::Type::tkOpenBracket),
    std::make_pair("]", Token::Type::tkClosedBracket),
    std::make_pair("(", Token::Type::tkOpenParenthesis),
    std::make_pair(")", Token::Type::tkClosedParenthesis),
    std::make_pair("{", Token::Type::tkOpenCurlyBrace),
    std::make_pair("}", Token::Type::tkClosedCurlyBrace),
    std::make_pair(";", Token::Type::tkSemicolon),
    std::make_pair(":=", Token::Type::tkAssign),

    std::make_pair("int", Token::Type::tkInt),
    std::make_pair("in", Token::Type::tkIn),

    std::make_pair("..", Token::Type::tkRange),
    std::make_pair(".", Token::Type::tkDot),

    std::make_pair("=>", Token::Type::tkArrow),
    std::make_pair("=", Token::Type::tkEqual),

    std::make_pair(">=", Token::Type::tkGreaterEq),
    std::make_pair(">", Token::Type::tkGreater),

    std::make_pair("<=", Token::Type::tkLessEq),
    std::make_pair("<", Token::Type::tkLess),

    std::make_pair("/=", Token::Type::tkNotEqual),
    std::make_pair("/", Token::Type::tkDivide)
};

class Identifier : public Token {
public:
    std::string identifier;
};

class Integer : public Token {
public:
    long value;
};

class Real : public Token {
public:
    long double value;
};

class StringLiteral : public Token {
public:
    std::string value;
};

class Lexer {
public:
    static std::vector<std::shared_ptr<Token>> tokenize(const std::string& code);
};
