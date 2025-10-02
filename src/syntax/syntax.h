#pragma once

#include <mm_malloc.h>
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
class Body;

std::optional<std::shared_ptr<Body>> parseProgram(SyntaxContext& context, size_t& pos);

class Expression;

// Sep -> tkSemicolon | tkNewLine
bool parseSep(SyntaxContext& context, size_t& pos);

// AssignExpression -> tkAssign Expression
std::optional<std::shared_ptr<Expression>> parseAssignExpression(SyntaxContext& context, size_t& pos);

// Body -> <* { Statement Sep } *>
class Body {
public:
    std::vector<std::shared_ptr<Statement>> statements;
    std::optional<std::shared_ptr<Body>> parse(SyntaxContext& context, size_t& pos);
};

std::optional<std::shared_ptr<Body>> parseLoopBody(SyntaxContext& context, size_t& pos);

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
class Statement {
public:
    static std::optional<std::shared_ptr<Statement>> parse(SyntaxContext& context, size_t& pos);
    virtual ~Statement() = default;
};

// VarStatement -> tkVar [tkNewLine] tkIdent [ AssignExpression ]
//     { tkComma [tkNewLine] tkIdent [ AssignExpression ] }
class VarStatement : public Statement {
public:
    std::vector<std::pair<std::string, std::shared_ptr<Expression>>> definitions;
    static std::optional<std::shared_ptr<VarStatement>> parse(SyntaxContext& context, size_t& pos);
    virtual ~VarStatement() override = default;
};

// IfStatement -> tkIf < Expression > tkThen [tkNewLine] Body
//     [ tkElse [tkNewLine] Body ] tkEnd
class IfStatement : public Statement {
public:
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Body> doIfTrue;
    std::optional<std::shared_ptr<Body>> doIfFalse;
    static std::optional<std::shared_ptr<IfStatement>> parse(SyntaxContext& context, size_t& pos);
    virtual ~IfStatement() override = default;
};

// ShortIfStatement -> tkIf < Expression > tkArrow Statement
class ShortIfStatement : public Statement {
public:
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Body> doIfTrue;
    static std::optional<std::shared_ptr<ShortIfStatement>> parse(SyntaxContext& context, size_t& pos);
    virtual ~ShortIfStatement() override = default;
};

// WhileStatement -> tkWhile < Expression > LoopBody
class WhileStatement : public Statement {
public:
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Body> action;
    static std::optional<std::shared_ptr<WhileStatement>> parse(SyntaxContext& context, size_t& pos);
    virtual ~WhileStatement() override = default;
};

// ForStatement -> tkFor [ tkIdent tkIn ] < Expression > [ tkRange < Expression > ] LoopBody
class ForStatement : public Statement {
public:
    std::optional<std::shared_ptr<IdentifierToken>> optVariableName;
    std::shared_ptr<Expression> startOrList;
    std::optional<std::shared_ptr<Expression>> end;
    std::shared_ptr<Body> action;
    static std::optional<std::shared_ptr<ForStatement>> parse(SyntaxContext& context, size_t& pos);
    virtual ~ForStatement() override = default;
};

// ExitStatement -> tkExit
class ExitStatement : public Statement {
public:
    static std::optional<std::shared_ptr<ExitStatement>> parse(SyntaxContext& context, size_t& pos);
    virtual ~ExitStatement() override = default;
};

class Reference;

// AssignStatement -> Reference tkAssign Expression
class AssignStatement : public Statement {
public:
    std::shared_ptr<Reference> dest;
    std::shared_ptr<Expression> src;
    static std::optional<std::shared_ptr<AssignStatement>> parse(SyntaxContext& context, size_t& pos);
    virtual ~AssignStatement() override = default;
};

// PrintStatement -> tkPrint [ CommaExpressions ]
class PrintStatement : public Statement {
public:
    std::vector<std::shared_ptr<Expression>> expressions;
    static std::optional<std::shared_ptr<PrintStatement>> parse(SyntaxContext& context, size_t& pos);
    virtual ~PrintStatement() override = default;
};

// ReturnStatement -> tkReturn [ Expression ]
class ReturnStatement : public Statement {
public:
    std::optional<std::shared_ptr<Expression>> returnValue;
    static std::optional<std::shared_ptr<ReturnStatement>> parse(SyntaxContext& context, size_t& pos);
    virtual ~ReturnStatement() override = default;
};

// ExpressionStatement -> Expression
class ExpressionStatement : public Statement {
public:
    std::shared_ptr<Expression> expr;
    static std::optional<std::shared_ptr<ExpressionStatement>> parse(SyntaxContext& context, size_t& pos);
    virtual ~ExpressionStatement() override = default;
};

// EmptyStatement -> 
class EmptyStatement : public Statement {
public:
    // No parse because EmptyStatement::parse(...) would do nothing and always succeed
    virtual ~EmptyStatement() override = default;
};

// CommaExpressions -> Expression { tkComma Expression }
class CommaExpressions {
public:
    std::vector<std::shared_ptr<Expression>> expressions;
    std::optional<std::shared_ptr<CommaExpressions>> parse(SyntaxContext& context, size_t& pos);
};

// CommaIdents -> tkIdent { tkComma tkIdent }
class CommaIdents {
public:
    std::vector<std::shared_ptr<IdentifierToken>> idents;
    std::optional<std::shared_ptr<CommaIdents>> parse(SyntaxContext& context, size_t& pos);
};

// Accessor -> MemberAccessor | IndexAccessor
class Accessor {
public:
    static std::optional<std::shared_ptr<Accessor>> parse(SyntaxContext& context, size_t& pos);
    virtual ~Accessor() = default;
};

// MemberAccessor -> tkDot ( tkIdent | tkIntLiteral | ParenthesesExpression )
//                           a.value   a.2            a.(1 + i)
class MemberAccessor : public Accessor {
public:
    static std::optional<std::shared_ptr<MemberAccessor>> parse(SyntaxContext& context, size_t& pos);
    virtual ~MemberAccessor() override = default;
};

class IdentMemberAccessor : public MemberAccessor {
public:
    std::shared_ptr<IdentifierToken> name;
    static std::optional<std::shared_ptr<IdentMemberAccessor>> parse(SyntaxContext& context, size_t& pos);
    virtual ~IdentMemberAccessor() override = default;
};

class IntLiteralMemberAccessor : public MemberAccessor {
public:
    int index;
    static std::optional<std::shared_ptr<IntLiteralMemberAccessor>> parse(SyntaxContext& context, size_t& pos);
    virtual ~IntLiteralMemberAccessor() override = default;
};

class ParenMemberAccessor : public MemberAccessor {
public:
    std::shared_ptr<Expression> expr;
    static std::optional<std::shared_ptr<ParenMemberAccessor>> parse(SyntaxContext& context, size_t& pos);
    virtual ~ParenMemberAccessor() override = default;
};

// IndexAccessor -> tkOpenBracket < Expression > tkClosedBracket
class IndexAccessor : public Accessor {
public:
    std::shared_ptr<Expression> expressionInBrackets;
    static std::optional<std::shared_ptr<IndexAccessor>> parse(SyntaxContext& context, size_t& pos);
    virtual ~IndexAccessor() override = default;
};

// Reference -> tkIdent { Accessor }
class Reference {
public:
    std::string baseIdent;
    std::vector<std::shared_ptr<Accessor>> accessorChain;
    static std::optional<std::shared_ptr<Accessor>> parse(SyntaxContext& context, size_t& pos);
};

class XorOperand;
class OrOperand;
class AndOperand;
class Sum;
enum class BinaryRelation;
class Term;
class Unary;
class Primary;

// Expression -> XorOperand { tkXor XorOperand }
class Expression {  // value = XOR of elements
    std::vector<std::shared_ptr<XorOperand>> operands;
    static std::optional<std::shared_ptr<Expression>> parse(SyntaxContext& context, size_t& pos);
};

// XorOperand -> OrOperand { tkOr OrOperand }
class XorOperand {  // value = OR of elements
    std::vector<std::shared_ptr<OrOperand>> operands;
    static std::optional<std::shared_ptr<XorOperand>> parse(SyntaxContext& context, size_t& pos);
};

// OrOperand -> AndOperand { tkAnd AndOperand }
class OrOperand {  // value = AND of elements
    std::vector<std::shared_ptr<AndOperand>> operands;
    static std::optional<std::shared_ptr<OrOperand>> parse(SyntaxContext& context, size_t& pos);
};

// AndOperand -> Sum { BinaryRelation Sum }
class AndOperand {  // value = AND of comparisons of elements
    std::vector<std::shared_ptr<Sum>> operands;
    std::vector<BinaryRelation> operators;
    static std::optional<std::shared_ptr<AndOperand>> parse(SyntaxContext& context, size_t& pos);
};

// BinaryRelation -> tkLess | tkLessEq | tkGreater | tkGreaterEq | tkEqual | tkNotEqual
enum class BinaryRelation {
    Less, LessEq, Greater, GreaterEq, Equal, NotEqual
};
std::optional<BinaryRelation> parseBinaryRelation(SyntaxContext& context, size_t& pos);

// Sum -> Term { (tkPlus | tkMinus) Term }
class Sum {
public:
    enum class SumOperator {
        Plus, Minus
    };
    std::vector<std::shared_ptr<Term>> terms;
    std::vector<SumOperator> operators;
    static std::optional<std::shared_ptr<Sum>> parse(SyntaxContext& context, size_t& pos);
};

// Term -> Unary { (tkTimes | tkDivide) Unary }
class Term {
public:
    enum class TermOperator {
        Times, Divide
    };
    std::vector<std::shared_ptr<Term>> terms;
    std::vector<TermOperator> operators;
    static std::optional<std::shared_ptr<Term>> parse(SyntaxContext& context, size_t& pos);
};

class PrefixOperator;
class PostfixOperator;

// Unary -> {PrefixOperator} Primary {PostfixOperator}
class Unary {
public:
    std::vector<std::shared_ptr<PrefixOperator>> prefixOps;
    std::vector<std::shared_ptr<PostfixOperator>> postfixOps;
    std::shared_ptr<Primary> expr;
    static std::optional<std::shared_ptr<Unary>> parse(SyntaxContext& context, size_t& pos);
};

// Unary operator precedence:
// 1. function(args)  obj.field  arr[index]  // call & accessors
// 2. +num -num
// 3. obj is type
// 4. not value

// PrefixOperator -> tkNot | tkMinus | tkPlus
class PrefixOperator {
public:
    enum class PrefixOperatorKind {
        Not, Plus, Minus
    };
    int precedence();  // the less, the more priority
    static int precedence(PrefixOperatorKind op);
    static std::optional<std::shared_ptr<PrefixOperator>> parse(SyntaxContext& context, size_t& pos);
};

// PostfixOperator -> TypecheckOperator | Call | AccessorOperator
class PostfixOperator {
public:
    virtual int precedence() = 0;  // the less, the more priority
    static std::optional<std::shared_ptr<PostfixOperator>> parse(SyntaxContext& context, size_t& pos);
    virtual ~PostfixOperator() = default;
};

enum class TypeId;

// TypecheckOperator -> tkIs TypeId
class TypecheckOperator : public PostfixOperator {
public:
    TypeId typeId;
    virtual int precedence() override;  // = 3
    static std::optional<std::shared_ptr<TypecheckOperator>> parse(SyntaxContext& context, size_t& pos);
    virtual ~TypecheckOperator() = default;
};

// TypeId -> tkInt | tkReal | tkString | tkBool | tkNone | tkFunc
//     | tkOpenBracket tkClosedBracket | tkOpenCurlyBrace tkClosedCurlyBrace
enum class TypeId {
    Int, Real, String, Book, None, Func, Tuple, List
};
std::optional<TypeId> parseTypeId(SyntaxContext& context, size_t& pos);

// Call -> tkOpenParenthesis < [ CommaExpressions ] > tkClosedParenthesis
class Call : public PostfixOperator {
public:
    std::vector<std::shared_ptr<Expression>> args;
    virtual int precedence() override;  // = 1
    static std::optional<std::shared_ptr<Call>> parse(SyntaxContext& context, size_t& pos);
    virtual ~Call() = default;
};

// AccessorOperator -> Accessor
class AccessorOperator : public PostfixOperator {
public:
    std::shared_ptr<Accessor> accessor;
    virtual int precedence() override;  // = 1
    static std::optional<std::shared_ptr<AccessorOperator>> parse(SyntaxContext& context, size_t& pos);
    virtual ~AccessorOperator() = default;
};

// Primary -> PrimaryIdent | ParenthesesExpression | FuncLiteral | TokenLiteral | ArrayLiteral | TupleLiteral
class Primary {
public:
    static std::optional<std::shared_ptr<Primary>> parse(SyntaxContext& context, size_t& pos);
    virtual ~Primary() = default;
};

// PrimaryIdent -> tkIdent
class PrimaryIdent : public Primary {
public:
    std::shared_ptr<IdentifierToken> name;
    static std::optional<std::shared_ptr<PrimaryIdent>> parse(SyntaxContext& context, size_t& pos);
    virtual ~PrimaryIdent() = default;
};

// ParenthesesExpression -> tkOpenParenthesis < Expression > tkClosedParenthesis
class ParenthesesExpression : public Primary {
public:
    std::shared_ptr<Expression> expr;
    static std::optional<std::shared_ptr<ParenthesesExpression>> parse(SyntaxContext& context, size_t& pos);
    virtual ~ParenthesesExpression() = default;
};

// TupleLiteralElement -> [ tkIdent tkAssign ] Expression
class TupleLiteralElement {
public:
    std::optional<std::shared_ptr<IdentifierToken>> ident;
    std::shared_ptr<Expression> expression;
    std::optional<std::shared_ptr<TupleLiteralElement>> parse(SyntaxContext& context, size_t& pos);
};

// TupleLiteral -> tkOpenCurlyBrace < [ TupleLiteralElement { tkComma TupleLiteralElement } ] > tkClosedCurlyBrace
class TupleLiteral : public Primary {
public:
    std::vector<std::shared_ptr<TupleLiteralElement>> elements;
    std::optional<std::shared_ptr<TupleLiteral>> parse(SyntaxContext& context, size_t& pos);
    virtual ~TupleLiteral() override = default;
};

// FuncBody -> ShortFuncBody | LongFuncBody
class FuncBody {
public:
    static std::optional<std::shared_ptr<FuncBody>> parse(SyntaxContext& context, size_t& pos);
    virtual ~FuncBody() = default;
};

// ShortFuncBody -> tkArrow Expression
class ShortFuncBody : public FuncBody {
public:
    std::shared_ptr<Expression> expressionToReturn;
    static std::optional<std::shared_ptr<ShortFuncBody>> parse(SyntaxContext& context, size_t& pos);
    virtual ~ShortFuncBody() override = default;
};

// LongFuncBody -> tkIs Body tkEnd
class LongFuncBody : public FuncBody {
public:
    std::shared_ptr<Body> funcBody;
    static std::optional<std::shared_ptr<LongFuncBody>> parse(SyntaxContext& context, size_t& pos);
    virtual ~LongFuncBody() override = default;
};

// FuncLiteral -> tkFunc tkOpenParenthesis < [ CommaIdents ] > tkClosedParenthesis FuncBody
class FuncLiteral : public Primary {
public:
    std::vector<std::shared_ptr<IdentifierToken>> parameters;
    std::optional<std::shared_ptr<FuncBody>> funcBody;
    std::optional<std::shared_ptr<FuncLiteral>> parse(SyntaxContext& context, size_t& pos);
    virtual ~FuncLiteral() override = default;
};

// TokenLiteral -> tkStringLiteral | tkIntLiteral | tkRealLiteral | tkTrue | tkFalse | tkNone
class TokenLiteral : public Primary {
public:
    enum class TokenLiteralKind {
        String, Int, Real, True, False, None
    };
    TokenLiteralKind kind;
    // Yes, technically, `kind` is useless because `token->type` exists.
    // Still, it is nice when you have all the available options and nothing else.
    std::shared_ptr<Token> token;
    std::optional<std::shared_ptr<TokenLiteral>> parse(SyntaxContext& context, size_t& pos);
    virtual ~TokenLiteral() override = default;
};

// ArrayLiteral -> tkOpenBracket < [ CommaExpressions ] > tkClosedBracket
class ArrayLiteral : public Primary {
public:
    std::optional<std::shared_ptr<Expression>> items;
    std::optional<std::shared_ptr<ArrayLiteral>> parse(SyntaxContext& context, size_t& pos);
    virtual ~ArrayLiteral() override = default;
};

}

class SyntaxAnalyzer {
public:
    static std::optional<std::shared_ptr<ast::Body>> analyze(
        const std::vector<std::shared_ptr<Token>>& tokens,
        const std::shared_ptr<const locators::CodeFile>& file,
        complog::ICompilationLog& log);
};
