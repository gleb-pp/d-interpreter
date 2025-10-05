#pragma once

#include <optional>
#include <vector>
#include <memory>
#include "locators/locator.h"
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
    locators::SpanLocator MakeSpanLocator(size_t position, size_t length);
    // pastTheLastToken = index of the last token + 1. This is the value
    // of size_t& pos at the end of any class::parse method.
    locators::SpanLocator MakeSpanFromTokens(size_t firstToken, size_t pastTheLastToken);
    SyntaxContext(const std::vector<std::shared_ptr<Token>>& tokens,
                  const std::shared_ptr<const locators::CodeFile>& file);
};

namespace ast {
class IASTVisitor;

class ASTNode {
public:
    locators::SpanLocator pos;
    ASTNode(const locators::SpanLocator& pos);
    virtual void AcceptVisitor(IASTVisitor& vis) = 0;
    virtual ~ASTNode() = default;
};

class Statement;
class Body;

// PROGRAM -> <* [ { Statement Sep } Statement [Sep] ] *>
std::optional<std::shared_ptr<Body>> parseProgram(SyntaxContext& context, size_t& pos);

class Expression;

// Sep -> tkSemicolon | tkNewLine
bool parseSep(SyntaxContext& context, size_t& pos);

// AssignExpression -> tkAssign Expression
std::optional<std::shared_ptr<Expression>> parseAssignExpression(SyntaxContext& context, size_t& pos);

// Body -> <* { Statement Sep } *>
class Body : public ASTNode {
public:
    Body(const locators::SpanLocator& pos);
    Body(const locators::SpanLocator& pos, const std::vector<std::shared_ptr<Statement>>& statements);
    std::vector<std::shared_ptr<Statement>> statements;
    static std::optional<std::shared_ptr<Body>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~Body() override = default;
};

// LoopBody -> tkLoop [tkNewLine] Body tkEnd
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
class Statement : public ASTNode {
public:
    Statement(const locators::SpanLocator& pos);
    static std::optional<std::shared_ptr<Statement>> parse(SyntaxContext& context, size_t& pos);
    virtual ~Statement() override = default;
};

// VarStatement -> tkVar [tkNewLine] tkIdent [ AssignExpression ]
//     { tkComma [tkNewLine] tkIdent [ AssignExpression ] }
class VarStatement : public Statement {
public:
    VarStatement(const locators::SpanLocator& pos);
    std::vector<std::pair<std::string, std::shared_ptr<Expression>>> definitions;
    static std::optional<std::shared_ptr<VarStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~VarStatement() override = default;
};

// IfStatement -> tkIf < Expression > tkThen [tkNewLine] Body
//     [ tkElse [tkNewLine] Body ] tkEnd
class IfStatement : public Statement {
public:
    IfStatement(const locators::SpanLocator& pos);
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Body> doIfTrue;
    std::optional<std::shared_ptr<Body>> doIfFalse;
    static std::optional<std::shared_ptr<IfStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~IfStatement() override = default;
};

// ShortIfStatement -> tkIf < Expression > tkArrow Statement
class ShortIfStatement : public Statement {
public:
    ShortIfStatement(const locators::SpanLocator& pos);
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Body> doIfTrue;
    static std::optional<std::shared_ptr<ShortIfStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ShortIfStatement() override = default;
};

// WhileStatement -> tkWhile < Expression > LoopBody
class WhileStatement : public Statement {
public:
    WhileStatement(const locators::SpanLocator& pos);
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Body> action;
    static std::optional<std::shared_ptr<WhileStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~WhileStatement() override = default;
};

// ForStatement -> tkFor [ tkIdent tkIn ] < Expression > [ tkRange < Expression > ] [tkNewLine] LoopBody
class ForStatement : public Statement {
public:
    ForStatement(const locators::SpanLocator& pos);
    std::optional<std::shared_ptr<IdentifierToken>> optVariableName;
    std::shared_ptr<Expression> startOrList;
    std::optional<std::shared_ptr<Expression>> end;
    std::shared_ptr<Body> action;
    static std::optional<std::shared_ptr<ForStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ForStatement() override = default;
};

// ExitStatement -> tkExit
class ExitStatement : public Statement {
public:
    ExitStatement(const locators::SpanLocator& pos);
    static std::optional<std::shared_ptr<ExitStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ExitStatement() override = default;
};

class Reference;

// AssignStatement -> Reference tkAssign Expression
class AssignStatement : public Statement {
public:
    AssignStatement(const locators::SpanLocator& pos);
    std::shared_ptr<Reference> dest;
    std::shared_ptr<Expression> src;
    static std::optional<std::shared_ptr<AssignStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~AssignStatement() override = default;
};

// PrintStatement -> tkPrint [ CommaExpressions ]
class PrintStatement : public Statement {
public:
    PrintStatement(const locators::SpanLocator& pos);
    std::vector<std::shared_ptr<Expression>> expressions;
    static std::optional<std::shared_ptr<PrintStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~PrintStatement() override = default;
};

// ReturnStatement -> tkReturn [ Expression ]
class ReturnStatement : public Statement {
public:
    ReturnStatement(const locators::SpanLocator& pos);
    std::optional<std::shared_ptr<Expression>> returnValue;
    static std::optional<std::shared_ptr<ReturnStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ReturnStatement() override = default;
};

// ExpressionStatement -> Expression
class ExpressionStatement : public Statement {
public:
    ExpressionStatement(const locators::SpanLocator& pos);
    std::shared_ptr<Expression> expr;
    static std::optional<std::shared_ptr<ExpressionStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ExpressionStatement() override = default;
};

// EmptyStatement -> 
class EmptyStatement : public Statement {
public:
    EmptyStatement(const locators::SpanLocator& pos);
    // parse does nothing and always succeeds
    static std::shared_ptr<EmptyStatement> parse();
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~EmptyStatement() override = default;
};

// CommaExpressions -> Expression { tkComma Expression }
class CommaExpressions : public ASTNode {
public:
    CommaExpressions(const locators::SpanLocator& pos);
    std::vector<std::shared_ptr<Expression>> expressions;
    void AcceptVisitor(IASTVisitor& vis) override;
    std::optional<std::shared_ptr<CommaExpressions>> parse(SyntaxContext& context, size_t& pos);
    virtual ~CommaExpressions() override = default;
};

// CommaIdents -> tkIdent { tkComma tkIdent }
class CommaIdents : public ASTNode {
public:
    CommaIdents(const locators::SpanLocator& pos);
    std::vector<std::shared_ptr<IdentifierToken>> idents;
    void AcceptVisitor(IASTVisitor& vis) override;
    std::optional<std::shared_ptr<CommaIdents>> parse(SyntaxContext& context, size_t& pos);
    virtual ~CommaIdents() override = default;
};

// Accessor -> MemberAccessor | IndexAccessor
// MemberAccessor -> tkDot ( tkIdent | tkIntLiteral | ParenthesesExpression )
//                           a.value   a.2            a.(1 + i)
class Accessor : public ASTNode {
public:
    Accessor(const locators::SpanLocator& pos);
    static std::optional<std::shared_ptr<Accessor>> parse(SyntaxContext& context, size_t& pos);
    virtual ~Accessor() override = default;
};

class IdentMemberAccessor : public Accessor {
public:
    IdentMemberAccessor(const locators::SpanLocator& pos);
    std::shared_ptr<IdentifierToken> name;
    static std::optional<std::shared_ptr<IdentMemberAccessor>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~IdentMemberAccessor() override = default;
};

class IntLiteralMemberAccessor : public Accessor {
public:
    IntLiteralMemberAccessor(const locators::SpanLocator& pos);
    std::shared_ptr<IntegerToken> index;
    static std::optional<std::shared_ptr<IntLiteralMemberAccessor>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~IntLiteralMemberAccessor() override = default;
};

class ParenMemberAccessor : public Accessor {
public:
    ParenMemberAccessor(const locators::SpanLocator& pos);
    std::shared_ptr<Expression> expr;
    static std::optional<std::shared_ptr<ParenMemberAccessor>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ParenMemberAccessor() override = default;
};

// IndexAccessor -> tkOpenBracket < Expression > tkClosedBracket
class IndexAccessor : public Accessor {
public:
    IndexAccessor(const locators::SpanLocator& pos);
    std::shared_ptr<Expression> expressionInBrackets;
    static std::optional<std::shared_ptr<IndexAccessor>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~IndexAccessor() override = default;
};

// Reference -> tkIdent { Accessor }
class Reference : public ASTNode {
public:
    Reference(const locators::SpanLocator& pos);
    std::string baseIdent;
    std::vector<std::shared_ptr<Accessor>> accessorChain;
    void AcceptVisitor(IASTVisitor& vis) override;
    static std::optional<std::shared_ptr<Reference>> parse(SyntaxContext& context, size_t& pos);
    virtual ~Reference() override = default;
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
class Expression : public ASTNode {  // value = XOR of elements
public:
    Expression(const locators::SpanLocator& pos);
    std::vector<std::shared_ptr<XorOperand>> operands;
    void AcceptVisitor(IASTVisitor& vis) override;
    static std::optional<std::shared_ptr<Expression>> parse(SyntaxContext& context, size_t& pos);
    virtual ~Expression() override = default;
};

// XorOperand -> OrOperand { tkOr OrOperand }
class XorOperand : public ASTNode {  // value = OR of elements
public:
    XorOperand(const locators::SpanLocator& pos);
    std::vector<std::shared_ptr<OrOperand>> operands;
    void AcceptVisitor(IASTVisitor& vis) override;
    static std::optional<std::shared_ptr<XorOperand>> parse(SyntaxContext& context, size_t& pos);
    virtual ~XorOperand() override = default;
};

// OrOperand -> AndOperand { tkAnd AndOperand }
class OrOperand : public ASTNode {  // value = AND of elements
public:
    OrOperand(const locators::SpanLocator& pos);
    std::vector<std::shared_ptr<AndOperand>> operands;
    void AcceptVisitor(IASTVisitor& vis) override;
    static std::optional<std::shared_ptr<OrOperand>> parse(SyntaxContext& context, size_t& pos);
    virtual ~OrOperand() override = default;
};

// AndOperand -> Sum { BinaryRelation Sum }
class AndOperand : public ASTNode {  // value = AND of comparisons of elements
public:
    AndOperand(const locators::SpanLocator& pos);
    std::vector<std::shared_ptr<Sum>> operands;
    std::vector<BinaryRelation> operators;
    void AcceptVisitor(IASTVisitor& vis) override;
    static std::optional<std::shared_ptr<AndOperand>> parse(SyntaxContext& context, size_t& pos);
    virtual ~AndOperand() override = default;
};

// BinaryRelation -> tkLess | tkLessEq | tkGreater | tkGreaterEq | tkEqual | tkNotEqual
enum class BinaryRelation {
    Less, LessEq, Greater, GreaterEq, Equal, NotEqual
};
std::optional<BinaryRelation> parseBinaryRelation(SyntaxContext& context, size_t& pos);

// Sum -> Term { (tkPlus | tkMinus) Term }
class Sum : public ASTNode {
public:
    Sum(const locators::SpanLocator& pos);
    enum class SumOperator {
        Plus, Minus
    };
    std::vector<std::shared_ptr<Term>> terms;
    std::vector<SumOperator> operators;
    void AcceptVisitor(IASTVisitor& vis) override;
    static std::optional<std::shared_ptr<Sum>> parse(SyntaxContext& context, size_t& pos);
    virtual ~Sum() override = default;
};

// Term -> Unary { (tkTimes | tkDivide) Unary }
class Term : public ASTNode {
public:
    Term(const locators::SpanLocator& pos);
    enum class TermOperator {
        Times, Divide
    };
    std::vector<std::shared_ptr<Term>> terms;
    std::vector<TermOperator> operators;
    void AcceptVisitor(IASTVisitor& vis) override;
    static std::optional<std::shared_ptr<Term>> parse(SyntaxContext& context, size_t& pos);
    virtual ~Term() override = default;
};

class PrefixOperator;
class PostfixOperator;

// Unary -> {PrefixOperator} Primary {PostfixOperator}
class Unary : public ASTNode {
public:
    Unary(const locators::SpanLocator& pos);
    std::vector<std::shared_ptr<PrefixOperator>> prefixOps;
    std::vector<std::shared_ptr<PostfixOperator>> postfixOps;
    std::shared_ptr<Primary> expr;
    void AcceptVisitor(IASTVisitor& vis) override;
    static std::optional<std::shared_ptr<Unary>> parse(SyntaxContext& context, size_t& pos);
    virtual ~Unary() override = default;
};

// Unary operator precedence:
// 1. function(args)  obj.field  arr[index]  // call & accessors
// 2. +num -num
// 3. obj is type
// 4. not value

// PrefixOperator -> tkNot | tkMinus | tkPlus
class PrefixOperator : public ASTNode {
public:
    PrefixOperator(const locators::SpanLocator& pos);
    enum class PrefixOperatorKind {
        Not, Plus, Minus
    };
    PrefixOperatorKind kind;
    int precedence();  // the less, the more priority
    static int precedence(PrefixOperatorKind op);
    void AcceptVisitor(IASTVisitor& vis) override;
    static std::optional<std::shared_ptr<PrefixOperator>> parse(SyntaxContext& context, size_t& pos);
    virtual ~PrefixOperator() override = default;
};

// PostfixOperator -> TypecheckOperator | Call | AccessorOperator
class PostfixOperator : public ASTNode {
public:
    PostfixOperator(const locators::SpanLocator& pos);
    virtual int precedence() = 0;  // the less, the more priority
    static std::optional<std::shared_ptr<PostfixOperator>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~PostfixOperator() override = default;
};

enum class TypeId;

// TypecheckOperator -> tkIs TypeId
class TypecheckOperator : public PostfixOperator {
public:
    TypecheckOperator(const locators::SpanLocator& pos);
    TypeId typeId;
    virtual int precedence() override;  // = 3
    static std::optional<std::shared_ptr<TypecheckOperator>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~TypecheckOperator() override = default;
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
    Call(const locators::SpanLocator& pos);
    std::vector<std::shared_ptr<Expression>> args;
    virtual int precedence() override;  // = 1
    static std::optional<std::shared_ptr<Call>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~Call() override = default;
};

// AccessorOperator -> Accessor
class AccessorOperator : public PostfixOperator {
public:
    AccessorOperator(const locators::SpanLocator& pos);
    std::shared_ptr<Accessor> accessor;
    virtual int precedence() override;  // = 1
    static std::optional<std::shared_ptr<AccessorOperator>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~AccessorOperator() override = default;
};

// Primary -> PrimaryIdent | ParenthesesExpression | FuncLiteral | TokenLiteral | ArrayLiteral | TupleLiteral
class Primary : public ASTNode {
public:
    Primary(const locators::SpanLocator& pos);
    static std::optional<std::shared_ptr<Primary>> parse(SyntaxContext& context, size_t& pos);
    virtual ~Primary() override = default;
};

// PrimaryIdent -> tkIdent
class PrimaryIdent : public Primary {
public:
    PrimaryIdent(const locators::SpanLocator& pos);
    std::shared_ptr<IdentifierToken> name;
    static std::optional<std::shared_ptr<PrimaryIdent>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~PrimaryIdent() override = default;
};

// ParenthesesExpression -> tkOpenParenthesis < Expression > tkClosedParenthesis
class ParenthesesExpression : public Primary {
public:
    ParenthesesExpression(const locators::SpanLocator& pos);
    std::shared_ptr<Expression> expr;
    static std::optional<std::shared_ptr<ParenthesesExpression>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ParenthesesExpression() override = default;
};

// TupleLiteralElement -> [ tkIdent tkAssign ] Expression
class TupleLiteralElement : public ASTNode {
public:
    TupleLiteralElement(const locators::SpanLocator& pos);
    std::optional<std::shared_ptr<IdentifierToken>> ident;
    std::shared_ptr<Expression> expression;
    void AcceptVisitor(IASTVisitor& vis) override;
    std::optional<std::shared_ptr<TupleLiteralElement>> parse(SyntaxContext& context, size_t& pos);
    virtual ~TupleLiteralElement() override = default;
};

// TupleLiteral -> tkOpenCurlyBrace < [ TupleLiteralElement { tkComma TupleLiteralElement } ] > tkClosedCurlyBrace
class TupleLiteral : public Primary {
public:
    TupleLiteral(const locators::SpanLocator& pos);
    std::vector<std::shared_ptr<TupleLiteralElement>> elements;
    std::optional<std::shared_ptr<TupleLiteral>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~TupleLiteral() override = default;
};

// FuncBody -> ShortFuncBody | LongFuncBody
class FuncBody : public ASTNode {
public:
    FuncBody(const locators::SpanLocator& pos);
    static std::optional<std::shared_ptr<FuncBody>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~FuncBody() override = default;
};

// ShortFuncBody -> tkArrow Expression
class ShortFuncBody : public FuncBody {
public:
    ShortFuncBody(const locators::SpanLocator& pos);
    std::shared_ptr<Expression> expressionToReturn;
    static std::optional<std::shared_ptr<ShortFuncBody>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ShortFuncBody() override = default;
};

// LongFuncBody -> tkIs Body tkEnd
class LongFuncBody : public FuncBody {
public:
    LongFuncBody(const locators::SpanLocator& pos);
    std::shared_ptr<Body> funcBody;
    static std::optional<std::shared_ptr<LongFuncBody>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~LongFuncBody() override = default;
};

// FuncLiteral -> tkFunc tkOpenParenthesis < [ CommaIdents ] > tkClosedParenthesis FuncBody
class FuncLiteral : public Primary {
public:
    FuncLiteral(const locators::SpanLocator& pos);
    std::vector<std::shared_ptr<IdentifierToken>> parameters;
    std::optional<std::shared_ptr<FuncBody>> funcBody;
    std::optional<std::shared_ptr<FuncLiteral>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~FuncLiteral() override = default;
};

// TokenLiteral -> tkStringLiteral | tkIntLiteral | tkRealLiteral | tkTrue | tkFalse | tkNone
class TokenLiteral : public Primary {
public:
    TokenLiteral(const locators::SpanLocator& pos);
    enum class TokenLiteralKind {
        String, Int, Real, True, False, None
    };
    TokenLiteralKind kind;
    // Yes, technically, `kind` is useless because `token->type` exists.
    // Still, it is nice when you have all the available options and nothing else.
    std::shared_ptr<Token> token;
    std::optional<std::shared_ptr<TokenLiteral>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~TokenLiteral() override = default;
};

// ArrayLiteral -> tkOpenBracket < [ CommaExpressions ] > tkClosedBracket
class ArrayLiteral : public Primary {
public:
    ArrayLiteral(const locators::SpanLocator& pos);
    std::optional<std::shared_ptr<Expression>> items;
    std::optional<std::shared_ptr<ArrayLiteral>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ArrayLiteral() override = default;
};

// This is needed for the **demo** program!!! I promise!
class IASTVisitor {
public:
    virtual void VisitBody(Body& node) = 0;
    virtual void VisitVarStatement(VarStatement& node) = 0;
    virtual void VisitIfStatement(IfStatement& node) = 0;
    virtual void VisitShortIfStatement(ShortIfStatement& node) = 0;
    virtual void VisitWhileStatement(WhileStatement& node) = 0;
    virtual void VisitForStatement(ForStatement& node) = 0;
    virtual void VisitExitStatement(ExitStatement& node) = 0;
    virtual void VisitAssignStatement(AssignStatement& node) = 0;
    virtual void VisitPrintStatement(PrintStatement& node) = 0;
    virtual void VisitReturnStatement(ReturnStatement& node) = 0;
    virtual void VisitExpressionStatement(ExpressionStatement& node) = 0;
    virtual void VisitEmptyStatement(EmptyStatement& node) = 0;
    virtual void VisitCommaExpressions(CommaExpressions& node) = 0;
    virtual void VisitCommaIdents(CommaIdents& node) = 0;
    virtual void VisitIdentMemberAccessor(IdentMemberAccessor& node) = 0;
    virtual void VisitIntLiteralMemberAccessor(IntLiteralMemberAccessor& node) = 0;
    virtual void VisitParenMemberAccessor(ParenMemberAccessor& node) = 0;
    virtual void VisitIndexAccessor(IndexAccessor& node) = 0;
    virtual void VisitReference(Reference& node) = 0;
    virtual void VisitExpression(Expression& node) = 0;
    virtual void VisitXorOperand(XorOperand& node) = 0;
    virtual void VisitOrOperand(OrOperand& node) = 0;
    virtual void VisitAndOperand(AndOperand& node) = 0;
    virtual void VisitSum(Sum& node) = 0;
    virtual void VisitTerm(Term& node) = 0;
    virtual void VisitUnary(Unary& node) = 0;
    virtual void VisitPrefixOperator(PrefixOperator& node) = 0;
    virtual void VisitTypecheckOperator(TypecheckOperator& node) = 0;
    virtual void VisitCall(Call& node) = 0;
    virtual void VisitAccessorOperator(AccessorOperator& node) = 0;
    virtual void VisitPrimaryIdent(PrimaryIdent& node) = 0;
    virtual void VisitParenthesesExpression(ParenthesesExpression& node) = 0;
    virtual void VisitTupleLiteralElement(TupleLiteralElement& node) = 0;
    virtual void VisitTupleLiteral(TupleLiteral& node) = 0;
    virtual void VisitShortFuncBody(ShortFuncBody& node) = 0;
    virtual void VisitLongFuncBody(LongFuncBody& node) = 0;
    virtual void VisitFuncLiteral(FuncLiteral& node) = 0;
    virtual void VisitTokenLiteral(TokenLiteral& node) = 0;
    virtual void VisitArrayLiteral(ArrayLiteral& node) = 0;
    virtual ~IASTVisitor() = default;
};
}

class SyntaxAnalyzer {
public:
    static std::optional<std::shared_ptr<ast::Body>> analyze(
        const std::vector<std::shared_ptr<Token>>& tokens,
        const std::shared_ptr<const locators::CodeFile>& file,
        complog::ICompilationLog& log);
};
