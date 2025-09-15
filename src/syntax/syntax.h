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

bool parseSep(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos,
              SyntaxErrorReport& report);

std::optional<std::shared_ptr<Expression>> parseAssignExpression(
    const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos,
    SyntaxErrorReport& report
);

bool parseExit(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos,
               SyntaxErrorReport& report);

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
    std::optional<std::shared_ptr<Statement>> parse(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos,
                                                   SyntaxErrorReport& report);
    virtual ~Statement() = default;
};

class VarStatement : public Statement {
public:

    virtual ~VarStatement() override = default;
};

Statement -> VarStatement // var a := 3
    | IfStatement         // if a = 3 then ... else ...
    | ShortIfStatement    // if a = 3 => ...
    | WhileStatement      // while ... loop ... end
    | ForStatement        // for i in start..stop loop ... end
    | ExitStatement       // exit
    | AssignStatement     // a := 3
    | PrintStatement      // print a, b, "c"
    | ReturnStatement     // return a + 4
    | ExpressionStatement // myObj.method(a, a + 1)
    | EmptyStatement      // ;

VarStatement -> tkVar [tkNewLine] tkIdent [ AssignExpression ]
    { tkComma [tkNewLine] tkIdent [ AssignExpression ] }

IfStatement -> tkIf < Expression > tkThen [tkNewLine] Body
    [ tkElse [tkNewLine] Body ] tkEnd

ShortIfStatement -> tkIf < Expression > tkArrow Statement

WhileStatement -> tkWhile < Expression > LoopBody

ForStatement -> tkFor [ Reference tkIn ] < Expression > [ tkRange < Expression > ] LoopBody

AssignStatement -> Reference tkAssign Expression

PrintStatement -> tkPrint [ CommaExpressions ]

CommaExpressions -> Expression { tkComma Expression }

ReturnStatement -> tkReturn [ Expression ]

ExpressionStatement -> Expression

EmptyStatement -> 

Reference -> tkIdent { Accessor }

Accessor -> MemberAccessor | IndexAccessor

MemberAccessor -> tkDot ( tkIdent | tkIntLiteral | ParenthesesExpression )
//                        a.value   a.2            a.(1 + i)

IndexAccessor -> tkOpenBracket < Expression > tkClosedBracket

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

Expression -> XorOperand { tkXor XorOperand }
XorOperand -> OrOperand { tkOr OrOperand }
OrOperand -> AndOperand { tkAnd AndOperand }
AndOperand -> Sum { BinaryRelation Sum }
BinaryRelation -> tkLess | tkLessEq
    | tkGreater | tkGreaterEq
    | tkEqual | tkNotEqual
Sum -> Term { (tkPlus | tkMinus) Term }
Term -> Unary { (tkTimes | tkDivide) Unary }

Unary -> {tkNot} TypecheckUnary
TypecheckUnary -> MinusPlusUnary { tkIs TypeId }
TypeId -> tkInt | tkReal | tkString | tkBool | tkNone | tkFunc
    | tkOpenBracket tkClosedBracket | tkOpenCurlyBrace tkClosedCurlyBrace
MinusPlusUnary -> {tkPlus | tkMinus} AccessorCallUnary
AccessorCallUnary -> Primary { Accessor | Call }
Call -> tkOpenParenthesis < [ CommaExpressions ] > tkClosedParenthesis

Primary -> PrimaryIdent | ParenthesesExpression | FuncLiteral | TokenLiteral | ArrayLiteral | TupleLiteral
PrimaryIdent -> tkIdent
ParenthesesExpression -> tkOpenParenthesis < Expression > tkClosedParenthesis
TokenLiteral -> tkStringLiteral | tkIntLiteral | tkRealLiteral | tkTrue | tkFalse | tkNone
FuncLiteral -> tkFunc tkOpenParenthesis < [ CommaIdents ] > tkClosedParenthesis FuncBody
ArrayLiteral -> tkOpenBracket < [ CommaExpressions ] > tkClosedBracket
TupleLiteral -> tkOpenCurlyBrace < [ TupleLiteralElement { tkComma TupleLiteralElement } ] > tkClosedCurlyBrace
TupleLiteralElement -> [ tkIdent tkAssign ] Expression
FuncBody -> ShortFuncBody | LongFuncBody
ShortFuncBody -> tkArrow Expression
LongFuncBody -> tkIs Body tkEnd

}

class SyntaxAnalyzer {
public:
    static std::optional<std::shared_ptr<ast::Program>> analyze(
        const std::vector<std::shared_ptr<Token>>& tokens,
        const std::shared_ptr<const locators::CodeFile>& file,
        complog::ICompilationLog& log);
};
