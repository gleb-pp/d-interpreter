#pragma once

#include <optional>
#include <vector>
#include <memory>
#include "complog/CompilationLog.h"
#include "complog/CompilationMessage.h"
#include "lexer.h"

class SyntaxErrorReport {
public:
    size_t rightmostPos = 0;
    std::vector<std::shared_ptr<complog::CompilationMessage>> messages;
    void Report(size_t pos, const std::shared_ptr<complog::CompilationMessage>& msg);
};

struct SyntaxContext {
    const std::vector<std::shared_ptr<Token>> tokens;
    SyntaxErrorReport report;
    std::shared_ptr<const locators::CodeFile> file;
    locators::Locator MakeLocator(size_t pos) const;
};

namespace ast {
class Statement;

class Program {
public:
    std::vector<std::shared_ptr<Statement>> statements;
    static std::optional<std::shared_ptr<Program>> parse(
        const std::vector<std::shared_ptr<Token>>& tokens,
        const std::shared_ptr<const locators::CodeFile>& file,
        complog::ICompilationLog& log
    );
};

class Expression;

// Sep -> tkSemicolon | tkNewLine
bool parseSep(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos,
              SyntaxErrorReport& report);

// AssignExpression -> tkAssign Expression
std::optional<std::shared_ptr<Expression>> parseAssignExpression(
    const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos,
    SyntaxErrorReport& report
);

// Body -> <* { Statement Sep } *>
class Body {
public:
    std::vector<std::shared_ptr<Statement>> statements;
    std::optional<std::shared_ptr<Body>> parse(
        const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos,
        SyntaxErrorReport& report
    );
};

std::optional<std::shared_ptr<Body>> parseLoopBody(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos,
                                                   SyntaxErrorReport& report);

class Statement {
public:
    static std::optional<std::shared_ptr<Statement>> parse(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos,
                                                   SyntaxErrorReport& report);
    virtual ~Statement() = default;
};

class VarStatement : public Statement {
public:

    virtual ~VarStatement() override = default;
};

// Statement -> VarStatement // var a := 3
//     | IfStatement         // if a = 3 then ... else ...
//     | ShortIfStatement    // if a = 3 => ...
//     | WhileStatement      // while ... loop ... end
//     | ForStatement        // for i in start..stop loop ... end
//     | ExitStatement       // exit
//     | AssignStatement     // a := 3
//     | PrintStatement      // print a, b, "c"
//     | ReturnStatement     // return a + 4
//     | ExpressionStatement // myObj.method(a, a + 1)
//     | EmptyStatement      // ;

// VarStatement -> tkVar [tkNewLine] tkIdent [ AssignExpression ]
//     { tkComma [tkNewLine] tkIdent [ AssignExpression ] }

// IfStatement -> tkIf < Expression > tkThen [tkNewLine] Body
//     [ tkElse [tkNewLine] Body ] tkEnd

// ShortIfStatement -> tkIf < Expression > tkArrow Statement

// WhileStatement -> tkWhile < Expression > LoopBody

// ForStatement -> tkFor [ Reference tkIn ] < Expression > [ tkRange < Expression > ] LoopBody

// ExitStatement -> tkExit

// AssignStatement -> Reference tkAssign Expression

// PrintStatement -> tkPrint [ CommaExpressions ]

// ReturnStatement -> tkReturn [ Expression ]

// ExpressionStatement -> Expression

// EmptyStatement -> 

// CommaExpressions -> Expression { tkComma Expression }
class CommaExpressions {
public:
    std::vector<std::shared_ptr<Expression>> expressions;
    std::optional<std::shared_ptr<CommaExpressions>> parse(
        const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos,
        SyntaxErrorReport& report
    );
};

// Accessor -> MemberAccessor | IndexAccessor
class Accessor {
public:
    static std::optional<std::shared_ptr<Accessor>> parse(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos, SyntaxErrorReport& report);
    virtual ~Accessor() = default;
};

// MemberAccessor -> tkDot ( tkIdent | tkIntLiteral | ParenthesesExpression )
//                           a.value   a.2            a.(1 + i)
class MemberAccessor : public Accessor {
public:
    // TODO: attibutes
    static std::optional<std::shared_ptr<MemberAccessor>> parse(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos, SyntaxErrorReport& report);
    virtual ~MemberAccessor() override = default;
};

// IndexAccessor -> tkOpenBracket < Expression > tkClosedBracket
class IndexAccessor : public Accessor {
public:
    std::shared_ptr<Expression> expressionInBrackets;
    static std::optional<std::shared_ptr<IndexAccessor>> parse(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos, SyntaxErrorReport& report);
    virtual ~IndexAccessor() override = default;
};

// Reference -> tkIdent { Accessor }
std::optional<std::shared_ptr<Accessor>> parseReference(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos, SyntaxErrorReport& report);

// Binary operator precedence:
// 1. * /
// 2. + -
// 3. > >= < <= = /=
// 4. and
// 5. or
// 6. xor
// Unary operator precedence:
// 1. function(args)  obj.field  arr[index]  // call & accessors
// 2. +num -num
// 3. obj is type
// 4. not value

// Expression -> XorOperand { tkXor XorOperand }
// XorOperand -> OrOperand { tkOr OrOperand }
// OrOperand -> AndOperand { tkAnd AndOperand }
// AndOperand -> Sum { BinaryRelation Sum }
// BinaryRelation -> tkLess | tkLessEq
//     | tkGreater | tkGreaterEq
//     | tkEqual | tkNotEqual
// Sum -> Term { (tkPlus | tkMinus) Term }
// Term -> Unary { (tkTimes | tkDivide) Unary }

// Unary -> {tkNot} TypecheckUnary
// TypecheckUnary -> MinusPlusUnary { tkIs TypeId }
// TypeId -> tkInt | tkReal | tkString | tkBool | tkNone | tkFunc
//     | tkOpenBracket tkClosedBracket | tkOpenCurlyBrace tkClosedCurlyBrace
// MinusPlusUnary -> {tkPlus | tkMinus} AccessorCallUnary
// AccessorCallUnary -> Primary { Accessor | Call }
// Call -> tkOpenParenthesis < [ CommaExpressions ] > tkClosedParenthesis

// Primary -> PrimaryIdent | ParenthesesExpression | FuncLiteral | TokenLiteral | ArrayLiteral | TupleLiteral
// PrimaryIdent -> tkIdent
// ParenthesesExpression -> tkOpenParenthesis < Expression > tkClosedParenthesis
// TokenLiteral -> tkStringLiteral | tkIntLiteral | tkRealLiteral | tkTrue | tkFalse | tkNone
// FuncLiteral -> tkFunc tkOpenParenthesis < [ CommaIdents ] > tkClosedParenthesis FuncBody

// ArrayLiteral -> tkOpenBracket < [ CommaExpressions ] > tkClosedBracket
class ArrayLiteral {
public:
    std::optional<std::shared_ptr<CommaExpressions>> commaExpression;
    std::optional<std::shared_ptr<ArrayLiteral>> parse(
        const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos,
        SyntaxErrorReport& report
    );
};

// TupleLiteralElement -> [ tkIdent tkAssign ] Expression
class TupleLiteralElement {
public:
    std::optional<std::shared_ptr<IdentifierToken>> ident;
    std::shared_ptr<Expression> expression;
    std::optional<std::shared_ptr<TupleLiteralElement>> parse(
        const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos,
        SyntaxErrorReport& report
    );
};

// TupleLiteral -> tkOpenCurlyBrace < [ TupleLiteralElement { tkComma TupleLiteralElement } ] > tkClosedCurlyBrace
class TupleLiteral {
public:
    std::vector<std::shared_ptr<TupleLiteralElement>> elements;
    std::optional<std::shared_ptr<TupleLiteral>> parse(
        const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos,
        SyntaxErrorReport& report
    );
};

// FuncBody -> ShortFuncBody | LongFuncBody
class FuncBody {
public:
    static std::optional<std::shared_ptr<FuncBody>> parse(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos, SyntaxErrorReport& report);
    virtual ~FuncBody() = default;
};

// ShortFuncBody -> tkArrow Expression
class ShortFuncBody : public FuncBody {
public:
    std::shared_ptr<Expression> expressionToReturn;
    static std::optional<std::shared_ptr<ShortFuncBody>> parse(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos, SyntaxErrorReport& report);
    virtual ~ShortFuncBody() override = default;
};

// LongFuncBody -> tkIs Body tkEnd
class LongFuncBody : public FuncBody {
public:
    std::shared_ptr<Body> funcBody;
    static std::optional<std::shared_ptr<LongFuncBody>> parse(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos, SyntaxErrorReport& report);
    virtual ~LongFuncBody() override = default;
};

}

class SyntaxAnalyzer {
public:
    static std::optional<std::shared_ptr<ast::Program>> analyze(
        const std::vector<std::shared_ptr<Token>>& tokens,
        const std::shared_ptr<const locators::CodeFile>& file,
        complog::ICompilationLog& log);
};
