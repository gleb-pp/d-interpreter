#pragma once

#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include "complog/CompilationLog.h"
#include "complog/CompilationMessage.h"
#include "lexer.h"
#include "locators/CodeFile.h"
#include "locators/locator.h"

class WrongNumberOfOperatorsSupplied : public std::invalid_argument {
public:
    WrongNumberOfOperatorsSupplied(const std::string& astclass, size_t operandscount, size_t operatorscount);
    virtual ~WrongNumberOfOperatorsSupplied() override = default;
};

class SyntaxErrorReport {
    std::shared_ptr<const locators::CodeFile> file;
    size_t rightmostPos = 0;
    std::map<Token::Type, std::set<Token::Type>> unexpTokens;

public:
    SyntaxErrorReport(const std::shared_ptr<const locators::CodeFile>& file);
    void ReportUnexpectedToken(size_t pos, Token::Type expected, Token::Type found);
    std::vector<std::shared_ptr<complog::CompilationMessage>> MakeReport() const;
};

class TokenScanner {
public:
    class AutoBlock {
        friend TokenScanner;
        TokenScanner* tk;
        AutoBlock(TokenScanner* tk);
        bool success = false;

    public:
        void Success();
        ~AutoBlock();
    };

private:
    struct StackBlock {
        int StartIndex;
        int Index;
        bool IgnoreEoln;
        StackBlock(int index, bool ignoreEoln);
    };

    std::shared_ptr<const locators::CodeFile> codeFile;
    std::vector<std::shared_ptr<Token>> tokens;
    std::vector<StackBlock> stack;
    SyntaxErrorReport report;

    size_t StartOfToken(size_t index) const;
    size_t EndOfToken(size_t index) const;
    void SkipEolns();

public:
    TokenScanner(const std::vector<std::shared_ptr<Token>>& tokens,
                 const std::shared_ptr<const locators::CodeFile>& file);
    locators::Locator PositionInFile() const;
    locators::Locator StartPositionInFile() const;
    locators::SpanLocator ReadSinceStart() const;
    size_t Index() const;
    const std::vector<std::shared_ptr<Token>>& Tokens() const;
    void Start();
    void StartIgnoreEoln();
    void StartUseEoln();
    void EndFail();
    void EndSuccess();
    std::shared_ptr<Token> Peek();
    std::shared_ptr<Token> Read();
    void Advance(size_t count = 1);
    std::optional<std::shared_ptr<Token>> Read(Token::Type type);
    SyntaxErrorReport& Report();
    const SyntaxErrorReport& Report() const;
    AutoBlock AutoStart();
    AutoBlock AutoStartIgnoreEoln();
    AutoBlock AutoStartUseEoln();
};

struct SyntaxContext {
    TokenScanner tokens;
    complog::ICompilationLog* const compilationLog;
    SyntaxContext(const std::vector<std::shared_ptr<Token>>& tokens,
                  const std::shared_ptr<const locators::CodeFile>& file, complog::ICompilationLog& complog);
};

namespace ast {
class IASTVisitor;

class ASTNode : public std::enable_shared_from_this<ASTNode> {
public:
    locators::SpanLocator pos;
    ASTNode(const locators::SpanLocator& pos);
    virtual void AcceptVisitor(IASTVisitor& vis) = 0;
    virtual ~ASTNode() = default;
};

class Statement;
class Body;

// PROGRAM -> <* [ { Statement Sep } Statement [Sep] ] *>
std::optional<std::shared_ptr<Body>> parseProgram(SyntaxContext& context);

class Expression;

// Sep -> tkSemicolon | tkNewLine
bool parseSep(SyntaxContext& context);

// AssignExpression -> tkAssign Expression
std::optional<std::shared_ptr<Expression>> parseAssignExpression(SyntaxContext& context);

// Body -> <* { Statement Sep } *>
class Body : public ASTNode {
public:
    Body(const locators::SpanLocator& pos);
    Body(const locators::SpanLocator& pos, const std::vector<std::shared_ptr<Statement>>& statements);
    std::vector<std::shared_ptr<Statement>> statements;
    static std::optional<std::shared_ptr<Body>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~Body() override = default;
};

// LoopBody -> tkLoop [tkNewLine] Body tkEnd
std::optional<std::shared_ptr<Body>> parseLoopBody(SyntaxContext& context);

// Statement -> VarStatement // var a := 3
//     | IfStatement         // if a = 3 then ... else ...
//     | ShortIfStatement    // if a = 3 => ...
//     | WhileStatement      // while ... loop ... end
//     | ForStatement        // for i in start..stop loop ... end
//     | LoopStatement       // loop print "working..."; end
//     | ExitStatement       // exit
//     | AssignStatement     // a := 3
//     | PrintStatement      // print a, b, "c"
//     | ReturnStatement     // return a + 4
//     | ExpressionStatement // myObj.method(a, a + 1)
//     | EmptyStatement      // ;
class Statement : public ASTNode {
public:
    Statement(const locators::SpanLocator& pos);
    static std::optional<std::shared_ptr<Statement>> parse(SyntaxContext& context);
    virtual ~Statement() override = default;
};

// VarStatement -> tkVar [tkNewLine] tkIdent [ AssignExpression ]
//     { tkComma [tkNewLine] tkIdent [ AssignExpression ] }
class VarStatement : public Statement {
public:
    VarStatement(const locators::SpanLocator& pos);
    std::vector<std::pair<std::string, std::optional<std::shared_ptr<Expression>>>> definitions;
    static std::optional<std::shared_ptr<VarStatement>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~VarStatement() override = default;
};

// IfStatement -> tkIf < Expression > [tkNewLine] tkThen [tkNewLine] Body
//     [ tkElse [tkNewLine] Body ] tkEnd
class IfStatement : public Statement {
public:
    IfStatement(const locators::SpanLocator& pos, const std::shared_ptr<Expression>& condition,
                const std::shared_ptr<Body>& doIfTrue, const std::optional<std::shared_ptr<Body>>& doIfFalse);
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Body> doIfTrue;
    std::optional<std::shared_ptr<Body>> doIfFalse;
    static std::optional<std::shared_ptr<IfStatement>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~IfStatement() override = default;
};

// ShortIfStatement -> tkIf < Expression > [tkNewLine] kArrow [tkNewLine] Statement
class ShortIfStatement : public Statement {
public:
    ShortIfStatement(const locators::SpanLocator& pos, const std::shared_ptr<Expression>& condition,
                     const std::shared_ptr<Statement>& doIfTrue);
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Statement> doIfTrue;
    static std::optional<std::shared_ptr<ShortIfStatement>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ShortIfStatement() override = default;
};

// WhileStatement -> tkWhile < Expression > LoopBody
class WhileStatement : public Statement {
public:
    WhileStatement(const locators::SpanLocator& pos, const std::shared_ptr<Expression>& condition,
                   const std::shared_ptr<Body>& action);
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Body> action;
    static std::optional<std::shared_ptr<WhileStatement>> parse(SyntaxContext& context);
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
    static std::optional<std::shared_ptr<ForStatement>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ForStatement() override = default;
};

// LoopStatement -> LoopBody
class LoopStatement : public Statement {
public:
    LoopStatement(const locators::SpanLocator& pos, const std::shared_ptr<Body>& body);
    std::shared_ptr<Body> body;
    static std::optional<std::shared_ptr<LoopStatement>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~LoopStatement() override = default;
};

// ExitStatement -> tkExit
class ExitStatement : public Statement {
public:
    ExitStatement(const locators::SpanLocator& pos);
    static std::optional<std::shared_ptr<ExitStatement>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ExitStatement() override = default;
};

class Reference;

// AssignStatement -> Reference tkAssign Expression
class AssignStatement : public Statement {
public:
    AssignStatement(const locators::SpanLocator& pos, const std::shared_ptr<Reference>& dest,
                    const std::shared_ptr<Expression>& src);
    std::shared_ptr<Reference> dest;
    std::shared_ptr<Expression> src;
    static std::optional<std::shared_ptr<AssignStatement>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~AssignStatement() override = default;
};

// PrintStatement -> tkPrint [ CommaExpressions ]
class PrintStatement : public Statement {
public:
    PrintStatement(const locators::SpanLocator& pos, const std::vector<std::shared_ptr<Expression>>& expressions);
    std::vector<std::shared_ptr<Expression>> expressions;
    static std::optional<std::shared_ptr<PrintStatement>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~PrintStatement() override = default;
};

// ReturnStatement -> tkReturn [ Expression ]
class ReturnStatement : public Statement {
public:
    ReturnStatement(const locators::SpanLocator& pos, const std::optional<std::shared_ptr<Expression>>& returnValue);
    std::optional<std::shared_ptr<Expression>> returnValue;
    static std::optional<std::shared_ptr<ReturnStatement>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ReturnStatement() override = default;
};

// ExpressionStatement -> Expression
class ExpressionStatement : public Statement {
public:
    ExpressionStatement(const locators::SpanLocator& pos, const std::shared_ptr<Expression>& expr);
    std::shared_ptr<Expression> expr;
    static std::optional<std::shared_ptr<ExpressionStatement>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ExpressionStatement() override = default;
};

// CommaExpressions -> Expression { tkComma Expression }
class CommaExpressions : public ASTNode {
public:
    CommaExpressions(const locators::SpanLocator& pos, const std::vector<std::shared_ptr<Expression>>& expressions);
    std::vector<std::shared_ptr<Expression>> expressions;
    void AcceptVisitor(IASTVisitor& vis) override;
    static std::optional<std::shared_ptr<CommaExpressions>> parse(SyntaxContext& context);
    virtual ~CommaExpressions() override = default;
};

// CommaIdents -> tkIdent { tkComma tkIdent }
class CommaIdents : public ASTNode {
public:
    CommaIdents(const locators::SpanLocator& pos, const std::vector<std::shared_ptr<IdentifierToken>>& idents);
    std::vector<std::shared_ptr<IdentifierToken>> idents;
    void AcceptVisitor(IASTVisitor& vis) override;
    static std::optional<std::shared_ptr<CommaIdents>> parse(SyntaxContext& context);
    virtual ~CommaIdents() override = default;
};

// Accessor -> MemberAccessor | IndexAccessor
// MemberAccessor -> tkDot ( tkIdent | tkIntLiteral | ParenthesesExpression )
//                           a.value   a.2            a.(1 + i)
class Accessor : public ASTNode {
public:
    Accessor(const locators::SpanLocator& pos);
    static std::optional<std::shared_ptr<Accessor>> parse(SyntaxContext& context);
    virtual ~Accessor() override = default;
};

class IdentMemberAccessor : public Accessor {
public:
    IdentMemberAccessor(const locators::SpanLocator& pos, const std::shared_ptr<IdentifierToken>& name);
    std::shared_ptr<IdentifierToken> name;
    static std::optional<std::shared_ptr<IdentMemberAccessor>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~IdentMemberAccessor() override = default;
};

class IntLiteralMemberAccessor : public Accessor {
public:
    IntLiteralMemberAccessor(const locators::SpanLocator& pos, const std::shared_ptr<IntegerToken>& index);
    std::shared_ptr<IntegerToken> index;
    static std::optional<std::shared_ptr<IntLiteralMemberAccessor>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~IntLiteralMemberAccessor() override = default;
};

class ParenMemberAccessor : public Accessor {
public:
    ParenMemberAccessor(const locators::SpanLocator& pos, const std::shared_ptr<Expression>& expr);
    std::shared_ptr<Expression> expr;
    static std::optional<std::shared_ptr<ParenMemberAccessor>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ParenMemberAccessor() override = default;
};

// IndexAccessor -> tkOpenBracket < Expression > tkClosedBracket
class IndexAccessor : public Accessor {
public:
    IndexAccessor(const locators::SpanLocator& pos, const std::shared_ptr<Expression>& expressionInBrackets);
    std::shared_ptr<Expression> expressionInBrackets;
    static std::optional<std::shared_ptr<IndexAccessor>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~IndexAccessor() override = default;
};

// Reference -> tkIdent { Accessor }
class Reference : public ASTNode {
public:
    Reference(const locators::SpanLocator& pos, const std::string& baseIdent,
              const std::vector<std::shared_ptr<Accessor>>& accessorChain);
    std::string baseIdent;
    std::vector<std::shared_ptr<Accessor>> accessorChain;
    void AcceptVisitor(IASTVisitor& vis) override;
    static std::optional<std::shared_ptr<Reference>> parse(SyntaxContext& context);
    virtual ~Reference() override = default;
};

class OrOperator;
class AndOperator;
class BinaryRelation;
class Sum;
enum class BinaryRelationOperator;
class Term;
class Unary;
class Primary;

class Expression : public ASTNode {
public:
    Expression(const locators::SpanLocator& pos);
    static std::optional<std::shared_ptr<Expression>> parse(SyntaxContext& context,
                                                            int max_precedence = std::numeric_limits<int>::max());
    virtual ~Expression() override = default;
};

class XorOperator : public Expression {  // value = XOR of elements
public:
    XorOperator(const std::vector<std::shared_ptr<Expression>>& operands);
    std::vector<std::shared_ptr<Expression>> operands;
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~XorOperator() override = default;
};

class OrOperator : public Expression {  // value = OR of elements
public:
    OrOperator(const std::vector<std::shared_ptr<Expression>>& operands);
    std::vector<std::shared_ptr<Expression>> operands;
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~OrOperator() override = default;
};

class AndOperator : public Expression {  // value = AND of elements
public:
    AndOperator(const std::vector<std::shared_ptr<Expression>>& operands);
    std::vector<std::shared_ptr<Expression>> operands;
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~AndOperator() override = default;
};

class BinaryRelation : public Expression {  // value = AND of comparisons of elements
public:
    BinaryRelation(const std::vector<std::shared_ptr<Expression>>& operands,
                   const std::vector<BinaryRelationOperator>& operators);
    std::vector<std::shared_ptr<Expression>> operands;
    std::vector<BinaryRelationOperator> operators;
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~BinaryRelation() override = default;
};

// BinaryRelationOperator -> tkLess | tkLessEq | tkGreater | tkGreaterEq | tkEqual | tkNotEqual
enum class BinaryRelationOperator { Less, LessEq, Greater, GreaterEq, Equal, NotEqual };
std::optional<BinaryRelationOperator> parseBinaryRelationOperator(SyntaxContext& context);

class Sum : public Expression {
public:
    enum class SumOperator { Plus, Minus };
    Sum(const std::vector<std::shared_ptr<Expression>>& terms, const std::vector<SumOperator>& operators);
    std::vector<std::shared_ptr<Expression>> terms;
    std::vector<SumOperator> operators;
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~Sum() override = default;
};

class Term : public Expression {
public:
    enum class TermOperator { Times, Divide };
    Term(const std::vector<std::shared_ptr<Expression>>& unaries, const std::vector<TermOperator>& operators);
    std::vector<std::shared_ptr<Expression>> unaries;
    std::vector<TermOperator> operators;
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~Term() override = default;
};

// UnaryNot -> tkNot Expression(precedence < And::precedence)
class UnaryNot : public Expression {
public:
    std::shared_ptr<Expression> nested;
    UnaryNot(const locators::SpanLocator& pos, const std::shared_ptr<Expression>& nested);
    static std::optional<std::shared_ptr<UnaryNot>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~UnaryNot() override = default;
};

class PrefixOperator;
class PostfixOperator;

// Unary -> {PrefixOperator} Primary {PostfixOperator}
class Unary : public Expression {
public:
    Unary(const locators::SpanLocator& pos, const std::vector<std::shared_ptr<PrefixOperator>>& prefixOps,
          const std::vector<std::shared_ptr<PostfixOperator>>& postfixOps, const std::shared_ptr<Expression>& expr);
    std::vector<std::shared_ptr<PrefixOperator>> prefixOps;
    std::vector<std::shared_ptr<PostfixOperator>> postfixOps;
    std::shared_ptr<Expression> expr;
    void AcceptVisitor(IASTVisitor& vis) override;
    static std::optional<std::shared_ptr<Unary>> parse(SyntaxContext& context);
    virtual ~Unary() override = default;
};

// Unary operator precedence:
// 1. function(args)  obj.field  arr[index]  // call & accessors
// 2. +num -num
// 3. obj is type

// PrefixOperator -> tkNot | tkMinus | tkPlus
class PrefixOperator : public ASTNode {
public:
    enum class PrefixOperatorKind { Plus, Minus };
    PrefixOperator(const locators::SpanLocator& pos, PrefixOperatorKind kind);
    PrefixOperatorKind kind;
    int precedence();  // the less, the more priority; always '2'
    void AcceptVisitor(IASTVisitor& vis) override;
    static std::optional<std::shared_ptr<PrefixOperator>> parse(SyntaxContext& context);
    virtual ~PrefixOperator() override = default;
};

// PostfixOperator -> TypecheckOperator | Call | AccessorOperator
class PostfixOperator : public ASTNode {
public:
    PostfixOperator(const locators::SpanLocator& pos);
    virtual int precedence() = 0;  // the less, the more priority
    static std::optional<std::shared_ptr<PostfixOperator>> parse(SyntaxContext& context);
    virtual ~PostfixOperator() override = default;
};

enum class TypeId;

// TypecheckOperator -> tkIs TypeId
class TypecheckOperator : public PostfixOperator {
public:
    TypecheckOperator(const locators::SpanLocator& pos, TypeId typeId);
    TypeId typeId;
    virtual int precedence() override;  // = 3
    static std::optional<std::shared_ptr<TypecheckOperator>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~TypecheckOperator() override = default;
};

// TypeId -> tkInt | tkReal | tkString | tkBool | tkNone | tkFunc
//     | tkOpenBracket tkClosedBracket | tkOpenCurlyBrace tkClosedCurlyBrace
enum class TypeId { Int, Real, String, Bool, None, Func, Tuple, List };
std::optional<TypeId> parseTypeId(SyntaxContext& context);

// Call -> tkOpenParenthesis < [ CommaExpressions ] > tkClosedParenthesis
class Call : public PostfixOperator {
public:
    Call(const locators::SpanLocator& pos, const std::vector<std::shared_ptr<Expression>>& args);
    std::vector<std::shared_ptr<Expression>> args;
    virtual int precedence() override;  // = 1
    static std::optional<std::shared_ptr<Call>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~Call() override = default;
};

// AccessorOperator -> Accessor
class AccessorOperator : public PostfixOperator {
public:
    AccessorOperator(const locators::SpanLocator& pos, const std::shared_ptr<Accessor>& accessor);
    std::shared_ptr<Accessor> accessor;
    virtual int precedence() override;  // = 1
    static std::optional<std::shared_ptr<AccessorOperator>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~AccessorOperator() override = default;
};

// Primary -> PrimaryIdent | ParenthesesExpression | FuncLiteral | TokenLiteral | ArrayLiteral | TupleLiteral
class Primary : public Expression {
public:
    Primary(const locators::SpanLocator& pos);
    static std::optional<std::shared_ptr<Primary>> parse(SyntaxContext& context);
    virtual ~Primary() override = default;
};

// PrimaryIdent -> tkIdent
class PrimaryIdent : public Primary {
public:
    PrimaryIdent(const locators::SpanLocator& pos, const std::shared_ptr<IdentifierToken>& name);
    std::shared_ptr<IdentifierToken> name;
    static std::optional<std::shared_ptr<PrimaryIdent>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~PrimaryIdent() override = default;
};

// ParenthesesExpression -> tkOpenParenthesis < Expression > tkClosedParenthesis
class ParenthesesExpression : public Primary {
public:
    ParenthesesExpression(const locators::SpanLocator& pos, const std::shared_ptr<Expression>& expr);
    std::shared_ptr<Expression> expr;
    static std::optional<std::shared_ptr<ParenthesesExpression>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ParenthesesExpression() override = default;
};

// TupleLiteralElement -> [ tkIdent tkAssign ] Expression
class TupleLiteralElement : public ASTNode {
public:
    TupleLiteralElement(const locators::SpanLocator& pos, const std::optional<std::shared_ptr<IdentifierToken>>& ident,
                        const std::shared_ptr<Expression>& expression);
    std::optional<std::shared_ptr<IdentifierToken>> ident;
    std::shared_ptr<Expression> expression;
    void AcceptVisitor(IASTVisitor& vis) override;
    static std::optional<std::shared_ptr<TupleLiteralElement>> parse(SyntaxContext& context);
    virtual ~TupleLiteralElement() override = default;
};

// TupleLiteral -> tkOpenCurlyBrace < [ TupleLiteralElement { tkComma TupleLiteralElement } ] > tkClosedCurlyBrace
class TupleLiteral : public Primary {
public:
    TupleLiteral(const locators::SpanLocator& pos, const std::vector<std::shared_ptr<TupleLiteralElement>>& elements);
    std::vector<std::shared_ptr<TupleLiteralElement>> elements;
    static std::optional<std::shared_ptr<TupleLiteral>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~TupleLiteral() override = default;
};

// FuncBody -> ShortFuncBody | LongFuncBody
class FuncBody : public ASTNode {
public:
    FuncBody(const locators::SpanLocator& pos);
    static std::optional<std::shared_ptr<FuncBody>> parse(SyntaxContext& context);
    virtual ~FuncBody() override = default;
};

// ShortFuncBody -> tkArrow Expression
class ShortFuncBody : public FuncBody {
public:
    ShortFuncBody(const locators::SpanLocator& pos, const std::shared_ptr<Expression>& expressionToReturn);
    std::shared_ptr<Expression> expressionToReturn;
    static std::optional<std::shared_ptr<ShortFuncBody>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ShortFuncBody() override = default;
};

// LongFuncBody -> tkIs Body tkEnd
class LongFuncBody : public FuncBody {
public:
    LongFuncBody(const locators::SpanLocator& pos, const std::shared_ptr<Body>& funcBody);
    std::shared_ptr<Body> funcBody;
    static std::optional<std::shared_ptr<LongFuncBody>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~LongFuncBody() override = default;
};

// FuncLiteral -> tkFunc tkOpenParenthesis < [ CommaIdents ] > tkClosedParenthesis FuncBody
class FuncLiteral : public Primary {
public:
    FuncLiteral(const locators::SpanLocator& pos, const std::vector<std::shared_ptr<IdentifierToken>>& parameters,
                const std::shared_ptr<FuncBody>& funcBody);
    std::vector<std::shared_ptr<IdentifierToken>> parameters;
    std::shared_ptr<FuncBody> funcBody;
    static std::optional<std::shared_ptr<FuncLiteral>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~FuncLiteral() override = default;
};

// TokenLiteral -> tkStringLiteral | tkIntLiteral | tkRealLiteral | tkTrue | tkFalse | tkNone
class TokenLiteral : public Primary {
public:
    enum class TokenLiteralKind { String, Int, Real, True, False, None };
    TokenLiteral(const locators::SpanLocator& pos, TokenLiteralKind kind, const std::shared_ptr<Token>& token);
    TokenLiteralKind kind;
    // Yes, technically, `kind` is useless because `token->type` exists.
    // Still, it is nice when you have all the available options and nothing else.
    std::shared_ptr<Token> token;
    static std::optional<std::shared_ptr<TokenLiteral>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~TokenLiteral() override = default;
};

// ArrayLiteral -> tkOpenBracket < [ CommaExpressions ] > tkClosedBracket
class ArrayLiteral : public Primary {
public:
    ArrayLiteral(const locators::SpanLocator& pos, const std::vector<std::shared_ptr<Expression>>& items);
    std::vector<std::shared_ptr<Expression>> items;
    static std::optional<std::shared_ptr<ArrayLiteral>> parse(SyntaxContext& context);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ArrayLiteral() override = default;
};

class IASTVisitor {
public:
    virtual void VisitBody(Body& node) = 0;
    virtual void VisitVarStatement(VarStatement& node) = 0;
    virtual void VisitIfStatement(IfStatement& node) = 0;
    virtual void VisitShortIfStatement(ShortIfStatement& node) = 0;
    virtual void VisitWhileStatement(WhileStatement& node) = 0;
    virtual void VisitForStatement(ForStatement& node) = 0;
    virtual void VisitLoopStatement(LoopStatement& node) = 0;
    virtual void VisitExitStatement(ExitStatement& node) = 0;
    virtual void VisitAssignStatement(AssignStatement& node) = 0;
    virtual void VisitPrintStatement(PrintStatement& node) = 0;
    virtual void VisitReturnStatement(ReturnStatement& node) = 0;
    virtual void VisitExpressionStatement(ExpressionStatement& node) = 0;
    virtual void VisitCommaExpressions(CommaExpressions& node) = 0;
    virtual void VisitCommaIdents(CommaIdents& node) = 0;
    virtual void VisitIdentMemberAccessor(IdentMemberAccessor& node) = 0;
    virtual void VisitIntLiteralMemberAccessor(IntLiteralMemberAccessor& node) = 0;
    virtual void VisitParenMemberAccessor(ParenMemberAccessor& node) = 0;
    virtual void VisitIndexAccessor(IndexAccessor& node) = 0;
    virtual void VisitReference(Reference& node) = 0;
    virtual void VisitXorOperator(XorOperator& node) = 0;
    virtual void VisitOrOperator(OrOperator& node) = 0;
    virtual void VisitAndOperator(AndOperator& node) = 0;
    virtual void VisitBinaryRelation(BinaryRelation& node) = 0;
    virtual void VisitSum(Sum& node) = 0;
    virtual void VisitTerm(Term& node) = 0;
    virtual void VisitUnary(Unary& node) = 0;
    virtual void VisitUnaryNot(UnaryNot& node) = 0;
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
    virtual void VisitCustom(ASTNode& node) = 0;
    virtual ~IASTVisitor() = default;
};
}  // namespace ast

class SyntaxAnalyzer {
public:
    static std::optional<std::shared_ptr<ast::Body>> analyze(const std::vector<std::shared_ptr<Token>>& tokens,
                                                             const std::shared_ptr<const locators::CodeFile>& file,
                                                             complog::ICompilationLog& log);
};
