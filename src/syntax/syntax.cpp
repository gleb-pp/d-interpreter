#include "syntax.h"
#include "complog/CompilationMessage.h"
#include "lexer.h"
#include "locators/locator.h"
#include <cstdlib>
#include <memory>
using namespace std;

// Auxiliary classes & functions

class UnexpectedTokenTypeError : public complog::CompilationMessage {
private:
    locators::Locator loc;
    Token::Type expected, found;

public:
    UnexpectedTokenTypeError(locators::Locator position, Token::Type expected, Token::Type found)
    : CompilationMessage(complog::Severity::Error(), "UnexpectedTokenTypeError"),
        loc(position), expected(expected), found(found) {}
    void WriteMessageToStream(ostream& out, const FormatOptions& options) const override {
        out << "Unexpected token at " << loc.Pretty() << "; expected " <<
            Token::TypeToString(expected) << ", but found " << Token::TypeToString(found) << ".\n";
    }
    vector<locators::Locator> Locators() const override {
        return { loc };
    }
    virtual ~UnexpectedTokenTypeError() override = default;
};

// Use this to check if a token is of the provided type. This automatically
// produces a compilation log message if the token is unexpected
bool AssertToken(SyntaxContext& context, size_t pos, Token::Type expected) {
    auto& token = *context.tokens[pos];
    if (token.type == expected)
        return true;
    size_t textpos = token.span.position;
    context.report.Report(textpos, make_shared<UnexpectedTokenTypeError>(
                          context.MakeLocator(textpos), expected, token.type));
    return false;
}

locators::Locator SyntaxContext::MakeLocator(size_t pos) const {
    return locators::Locator(file, pos);
}

locators::SpanLocator SyntaxContext::MakeSpanLocator(size_t position, size_t length) {
    return {file, position, length};
}

locators::SpanLocator SyntaxContext::MakeSpanFromTokens(size_t firsttoken, size_t pastthelast) {
    size_t start = tokens[firsttoken]->span.position;
    size_t end;
    if (pastthelast == firsttoken)
        end = start;
    else {
        auto& last = tokens[pastthelast - 1]->span;
        end = last.position + last.length;
    }
    return {file, start, end - start};
}

SyntaxContext::SyntaxContext(const vector<shared_ptr<Token>>& tokens,
        const shared_ptr<const locators::CodeFile>& file) : tokens(tokens), file(file) {}

// AST Node classes
// Every class in the ::parse implementation must:
// 1. On success, advance size_t& pos to the first unused token (after the last used token)
// 2. On failure, report unexpected tokens and what was expected
//    (perhaps by using AssertToken or context.report.Report)
// 3. On failure, reset size_t& pos to what it was at the start
// 4. Set SpanLocators (use context.MakeSpanFromTokens)
// If a class has an ::AcceptVisitor(vis) method, the implementation simply calls
//  vis.VisitMyClass(*this);

// Gleb, I encourage you to add constructors to the classes!

namespace ast {

ASTNode::ASTNode(const locators::SpanLocator& pos) : pos(pos) { }

optional<shared_ptr<Body>> parseProgram(SyntaxContext& context, size_t& pos) {
    vector<shared_ptr<Statement>> sts;
    size_t startpos = pos;
    while (true) {
        auto& tk = *context.tokens[pos];
        if (tk.type == Token::Type::tkEof) {
            ++pos;
            break;
        }
        auto optStatement = Statement::parse(context, pos);
        if (!optStatement.has_value()) {
            pos = startpos;
            return {};
        }
        sts.push_back(*optStatement);
        if (!parseSep(context, pos)) {
            if (!AssertToken(context, pos, Token::Type::tkEof))
                return {};
            else {
                ++pos;
                break;
            }
        }
    }
    auto res = make_shared<Body>(context.MakeSpanFromTokens(startpos, pos));
    res->statements = sts;
    return res;
}

bool parseSep(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkNewLine) && !AssertToken(context, pos, Token::Type::tkSemicolon))
        return false;
    ++pos;
    return true;
}

optional<shared_ptr<Expression>> parseAssignExpression(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkAssign)) return {};
    ++pos;
    return Expression::parse(context, pos);
}

Body::Body(const locators::SpanLocator& pos) : ASTNode(pos) { }
Body::Body(const locators::SpanLocator& pos, const vector<shared_ptr<Statement>>& statements)
    : ASTNode(pos), statements(statements) {}
optional<shared_ptr<Body>> Body::parse(SyntaxContext& context, size_t& pos) {
    vector<shared_ptr<Statement>> sts;
    size_t startpos = pos;
    while (true) {
        size_t prevpos = pos;
        auto optStatement = Statement::parse(context, pos);
        if (!optStatement.has_value()) {
            pos = prevpos;
            break;
        }
        if (!parseSep(context, pos)) {
            pos = prevpos;
            break;
        }
        sts.push_back(*optStatement);
    }
    auto res = make_shared<Body>(context.MakeSpanFromTokens(startpos, pos));
    res->statements = sts;
    return res;
}
void Body::AcceptVisitor(IASTVisitor& vis) { vis.VisitBody(*this); }

optional<shared_ptr<Body>> parseLoopBody(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkLoop))
        return {};
    size_t startpos = pos;
    ++pos;
    if (AssertToken(context, pos, Token::Type::tkNewLine)) ++pos;
    auto res = Body::parse(context, pos);
    if (!res.has_value()) {  // This is currently impossible. I am being explicit in case something changes.
        pos = startpos;
        return {};
    }
    if (!AssertToken(context, pos, Token::Type::tkEnd)) {
        pos = startpos;
        return {};
    }
    ++pos;
    return res;
}

Statement::Statement(const locators::SpanLocator& pos) : ASTNode(pos) {}
optional<shared_ptr<Statement>> Statement::parse(SyntaxContext& context, size_t& pos) {
    // It is tempting to write something like:
    //if (firstToken->type == Token::Type::tkVar)
    //    return VarStatement::parse(context, pos);
    // However, it is better to still run all variants of SmthStatement::parse(...)
    // because this will automatically handle error reports for the first token.
    // That is, if the first token is, for example, tkRange "..", which makes no sense,
    // the compiler should produce messages that at this position, 'var', 'if', 'for', etc. were expected.
    {
        auto res = VarStatement::parse(context, pos);
        if (res.has_value()) return res;
    }
    {
        auto res = IfStatement::parse(context, pos);
        if (res.has_value()) return res;
    }
    {
        auto res = ShortIfStatement::parse(context, pos);
        if (res.has_value()) return res;
    }
    {
        auto res = WhileStatement::parse(context, pos);
        if (res.has_value()) return res;
    }
    {
        auto res = ForStatement::parse(context, pos);
        if (res.has_value()) return res;
    }
    {
        auto res = LoopStatement::parse(context, pos);
        if (res.has_value()) return res;
    }
    {
        auto res = ExitStatement::parse(context, pos);
        if (res.has_value()) return res;
    }
    {
        auto res = AssignStatement::parse(context, pos);
        if (res.has_value()) return res;
    }
    {
        auto res = PrintStatement::parse(context, pos);
        if (res.has_value()) return res;
    }
    {
        auto res = ReturnStatement::parse(context, pos);
        if (res.has_value()) return res;
    }
    {
        auto res = ExpressionStatement::parse(context, pos);
        if (res.has_value()) return res;
    }
    return EmptyStatement::parse();
}

// These are to be implemented!

/*
// VarStatement -> tkVar [tkNewLine] tkIdent [ AssignExpression ]
//     { tkComma [tkNewLine] tkIdent [ AssignExpression ] }
class VarStatement : public Statement {
public:
    VarStatement(const locators::SpanLocator& pos);
    vector<pair<string, optional<shared_ptr<Expression>>>> definitions;
    static optional<shared_ptr<VarStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~VarStatement() override = default;
};

// IfStatement -> tkIf < Expression > tkThen [tkNewLine] Body
//     [ tkElse [tkNewLine] Body ] tkEnd
class IfStatement : public Statement {
public:
    IfStatement(const locators::SpanLocator& pos);
    shared_ptr<Expression> condition;
    shared_ptr<Body> doIfTrue;
    optional<shared_ptr<Body>> doIfFalse;
    static optional<shared_ptr<IfStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~IfStatement() override = default;
};

// ShortIfStatement -> tkIf < Expression > tkArrow Statement
class ShortIfStatement : public Statement {
public:
    ShortIfStatement(const locators::SpanLocator& pos);
    shared_ptr<Expression> condition;
    shared_ptr<Body> doIfTrue;
    static optional<shared_ptr<ShortIfStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ShortIfStatement() override = default;
};

// WhileStatement -> tkWhile < Expression > LoopBody
class WhileStatement : public Statement {
public:
    WhileStatement(const locators::SpanLocator& pos);
    shared_ptr<Expression> condition;
    shared_ptr<Body> action;
    static optional<shared_ptr<WhileStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~WhileStatement() override = default;
};
*/

// ForStatement -> tkFor [ tkIdent tkIn ] < Expression > [ tkRange < Expression > ] [tkNewLine] LoopBody
ForStatement::ForStatement(const locators::SpanLocator& pos) : Statement(pos) {}
//optional<shared_ptr<IdentifierToken>> optVariableName;
//shared_ptr<Expression> startOrList;
//optional<shared_ptr<Expression>> end;
//shared_ptr<Body> action;
optional<shared_ptr<ForStatement>> ForStatement::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkFor)) return {};
    size_t startpos = pos++;
    optional<shared_ptr<IdentifierToken>> optVarName;
    {
        if (AssertToken(context, pos, Token::Type::tkIdent)) {
            size_t prevpos = pos++;
            if (!AssertToken(context, pos, Token::Type::tkIn))
                pos = prevpos;
            else {
                optVarName = dynamic_pointer_cast<IdentifierToken>(context.tokens[prevpos]);
                ++pos;
            }
        }
    }
    auto startOrList = Expression::parse(context, pos);
    if (!startOrList.has_value()) {
        pos = startpos;
        return {};
    }
    optional<shared_ptr<Expression>> rangeEnd;
    if (AssertToken(context, pos, Token::Type::tkRange)) {
        size_t prevpos = pos;
        ++pos;
        rangeEnd = Expression::parse(context, pos);
        if (!rangeEnd.has_value()) pos = prevpos;
    }
    if (AssertToken(context, pos, Token::Type::tkNewLine)) ++pos;
    auto action = parseLoopBody(context, pos);
    if (!action.has_value()) {
        pos = startpos;
        return {};
    }
    auto res = make_shared<ForStatement>(context.MakeSpanFromTokens(startpos, pos));
    res->optVariableName = optVarName;
    res->startOrList = *startOrList;
    res->end = rangeEnd;
    res->action = *action;
    return res;
}
void ForStatement::AcceptVisitor(IASTVisitor& vis) {
    vis.VisitForStatement(*this);
}

/*
// LoopStatement -> LoopBody
class LoopStatement : public Statement {
public:
    LoopStatement(const locators::SpanLocator& pos);
    std::shared_ptr<Body> body;
    static std::optional<std::shared_ptr<LoopStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~LoopStatement() override = default;
};

// ExitStatement -> tkExit
class ExitStatement : public Statement {
public:
    ExitStatement(const locators::SpanLocator& pos);
    static optional<shared_ptr<ExitStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ExitStatement() override = default;
};

// AssignStatement -> Reference tkAssign Expression
class AssignStatement : public Statement {
public:
    AssignStatement(const locators::SpanLocator& pos);
    shared_ptr<Reference> dest;
    shared_ptr<Expression> src;
    static optional<shared_ptr<AssignStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~AssignStatement() override = default;
};

// PrintStatement -> tkPrint [ CommaExpressions ]
class PrintStatement : public Statement {
public:
    PrintStatement(const locators::SpanLocator& pos);
    vector<shared_ptr<Expression>> expressions;
    static optional<shared_ptr<PrintStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~PrintStatement() override = default;
};

// ReturnStatement -> tkReturn [ Expression ]
class ReturnStatement : public Statement {
public:
    ReturnStatement(const locators::SpanLocator& pos);
    optional<shared_ptr<Expression>> returnValue;
    static optional<shared_ptr<ReturnStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ReturnStatement() override = default;
};

// ExpressionStatement -> Expression
class ExpressionStatement : public Statement {
public:
    ExpressionStatement(const locators::SpanLocator& pos);
    shared_ptr<Expression> expr;
    static optional<shared_ptr<ExpressionStatement>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ExpressionStatement() override = default;
};

// EmptyStatement -> 
class EmptyStatement : public Statement {
public:
    EmptyStatement(const locators::SpanLocator& pos);
    // parse does nothing and always succeeds
    static shared_ptr<EmptyStatement> parse();
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~EmptyStatement() override = default;
};

// CommaExpressions -> Expression { tkComma Expression }
class CommaExpressions : public ASTNode {
public:
    CommaExpressions(const locators::SpanLocator& pos);
    vector<shared_ptr<Expression>> expressions;
    void AcceptVisitor(IASTVisitor& vis) override;
    optional<shared_ptr<CommaExpressions>> parse(SyntaxContext& context, size_t& pos);
    virtual ~CommaExpressions() override = default;
};

// CommaIdents -> tkIdent { tkComma tkIdent }
class CommaIdents : public ASTNode {
public:
    CommaIdents(const locators::SpanLocator& pos);
    vector<shared_ptr<IdentifierToken>> idents;
    void AcceptVisitor(IASTVisitor& vis) override;
    optional<shared_ptr<CommaIdents>> parse(SyntaxContext& context, size_t& pos);
    virtual ~CommaIdents() override = default;
};
*/

Accessor::Accessor(const locators::SpanLocator& pos) : ASTNode(pos) {}
optional<shared_ptr<Accessor>> Accessor::parse(SyntaxContext& context, size_t& pos) {
    {
        auto res = IdentMemberAccessor::parse(context, pos);
        if (res.has_value()) return res;
    }
    {
        auto res = IntLiteralMemberAccessor::parse(context, pos);
        if (res.has_value()) return res;
    }
    {
        auto res = ParenMemberAccessor::parse(context, pos);
        if (res.has_value()) return res;
    }
    {
        auto res = IndexAccessor::parse(context, pos);
        if (res.has_value()) return res;
    }
    return {};
}

IdentMemberAccessor::IdentMemberAccessor(const locators::SpanLocator& pos) : Accessor(pos) {}
optional<shared_ptr<IdentMemberAccessor>> IdentMemberAccessor::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkDot)) return {};
    size_t startpos = pos;
    ++pos;
    if (!AssertToken(context, pos, Token::Type::tkIdent)) {
        pos = startpos;
        return {};
    }
    auto tk = dynamic_pointer_cast<IdentifierToken>(context.tokens[pos++]);
    auto res = make_shared<IdentMemberAccessor>(context.MakeSpanFromTokens(startpos, pos));
    res->name = tk;
    return res;
}
void IdentMemberAccessor::AcceptVisitor(IASTVisitor& vis) {
    vis.VisitIdentMemberAccessor(*this);
}

IntLiteralMemberAccessor::IntLiteralMemberAccessor(const locators::SpanLocator& pos)
    : Accessor(pos) {}
optional<shared_ptr<IntLiteralMemberAccessor>> IntLiteralMemberAccessor::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkDot))
        return {};
    size_t startpos = pos;
    ++pos;
    if (!AssertToken(context, pos, Token::Type::tkIntLiteral)) {
        pos = startpos;
        return {};
    }
    auto tk = dynamic_pointer_cast<IntegerToken>(context.tokens[pos++]);
    auto res = make_shared<IntLiteralMemberAccessor>(context.MakeSpanFromTokens(startpos, pos));
    res->index = tk;
    return res;
}
void IntLiteralMemberAccessor::AcceptVisitor(IASTVisitor& vis) {
    vis.VisitIntLiteralMemberAccessor(*this);
}

ParenMemberAccessor::ParenMemberAccessor(const locators::SpanLocator& pos) : Accessor(pos) {}
optional<shared_ptr<ParenMemberAccessor>> ParenMemberAccessor::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkDot))
        return {};
    size_t startpos = pos;
    ++pos;
    auto expr = ParenthesesExpression::parse(context, pos);
    if (!expr.has_value()) {
        pos = startpos;
        return {};
    }
    auto res = make_shared<ParenMemberAccessor>(context.MakeSpanFromTokens(startpos, pos));
    res->expr = expr.value()->expr;
    return res;
}
void ParenMemberAccessor::AcceptVisitor(IASTVisitor& vis) {
    vis.VisitParenMemberAccessor(*this);
}

// IndexAccessor -> tkOpenBracket < Expression > tkClosedBracket
IndexAccessor::IndexAccessor(const locators::SpanLocator& pos) : Accessor(pos) {}
optional<shared_ptr<IndexAccessor>> IndexAccessor::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkOpenBracket)) return {};
    size_t startpos = pos++;
    auto expr = Expression::parse(context, pos);
    if (!expr.has_value()) {
        pos = startpos;
        return {};
    }
    if (!AssertToken(context, pos, Token::Type::tkOpenBracket)) {
        pos = startpos;
        return {};
    }
    ++pos;
    auto res = make_shared<IndexAccessor>(context.MakeSpanFromTokens(startpos, pos));
    res->expressionInBrackets = *expr;
    return res;
}
void IndexAccessor::AcceptVisitor(IASTVisitor& vis) {
    vis.VisitIndexAccessor(*this);
}

// Reference -> tkIdent { Accessor }
Reference::Reference(const locators::SpanLocator& pos) : ASTNode(pos) {}
optional<shared_ptr<Reference>> Reference::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkIdent)) return {};
    size_t startpos = pos;
    auto ident = dynamic_pointer_cast<IdentifierToken>(context.tokens[pos++]);
    vector<shared_ptr<Accessor>> chain;
    while (true) {
        auto acc = Accessor::parse(context, pos);
        if (!acc.has_value()) break;
        chain.push_back(*acc);
    }
    auto res = make_shared<Reference>(context.MakeSpanFromTokens(startpos, pos));
    res->accessorChain = chain;
    return res;
}
void Reference::AcceptVisitor(IASTVisitor& vis) {
    vis.VisitReference(*this);
}

// Expression -> OrOperator { tkXor OrOperator }
Expression::Expression(const locators::SpanLocator& pos) : ASTNode(pos) {}
optional<shared_ptr<Expression>> parse(SyntaxContext& context, size_t& pos) {
    vector<shared_ptr<OrOperator>> operands;
    size_t startpos = pos;
    bool first = true;
    while (true) {
        size_t prevpos = pos;
        if (!first) {
            if (!AssertToken(context, pos, Token::Type::tkXor))
                break;
            else
                ++pos;
        }
        first = false;
        auto op = OrOperator::parse(context, pos);
        if (!op.has_value()) {
            pos = prevpos;
            break;
        }
        operands.push_back(*op);
    }
    if (operands.empty()) return {};
    auto res = make_shared<Expression>(context.MakeSpanFromTokens(startpos, pos));
    res->operands = operands;
    return res;
}
void Expression::AcceptVisitor(IASTVisitor& vis) {
    vis.VisitExpression(*this);
}

/*
// OrOperator -> AndOperator { tkOr AndOperator }
class OrOperator : public ASTNode {  // value = OR of elements
public:
    vector<shared_ptr<AndOperator>> operands;
    void AcceptVisitor(IASTVisitor& vis) override;
    static optional<shared_ptr<OrOperator>> parse(SyntaxContext& context, size_t& pos);
    virtual ~OrOperator() override = default;
};

// AndOperator -> BinaryRelation { tkAnd BinaryRelation }
class AndOperator : public ASTNode {  // value = AND of elements
public:
    vector<shared_ptr<BinaryRelation>> operands;
    void AcceptVisitor(IASTVisitor& vis) override;
    static optional<shared_ptr<AndOperator>> parse(SyntaxContext& context, size_t& pos);
    virtual ~AndOperator() override = default;
};

// BinaryRelation -> Sum { BinaryRelationOperator Sum }
class BinaryRelation : public ASTNode {  // value = AND of comparisons of elements
public:
    vector<shared_ptr<Sum>> operands;
    vector<BinaryRelationOperator> operators;
    void AcceptVisitor(IASTVisitor& vis) override;
    static optional<shared_ptr<BinaryRelation>> parse(SyntaxContext& context, size_t& pos);
    virtual ~BinaryRelation() override = default;
};

// BinaryRelationOperator -> tkLess | tkLessEq | tkGreater | tkGreaterEq | tkEqual | tkNotEqual
optional<BinaryRelationOperator> parseBinaryRelationOperator(SyntaxContext& context, size_t& pos);

// Sum -> Term { (tkPlus | tkMinus) Term }
class Sum : public ASTNode {
public:
    Sum(const locators::SpanLocator& pos);
    vector<shared_ptr<Term>> terms;
    vector<SumOperator> operators;
    void AcceptVisitor(IASTVisitor& vis) override;
    static optional<shared_ptr<Sum>> parse(SyntaxContext& context, size_t& pos);
    virtual ~Sum() override = default;
};

// Term -> Unary { (tkTimes | tkDivide) Unary }
class Term : public ASTNode {
public:
    Term(const locators::SpanLocator& pos);
    vector<shared_ptr<Term>> terms;
    vector<TermOperator> operators;
    void AcceptVisitor(IASTVisitor& vis) override;
    static optional<shared_ptr<Term>> parse(SyntaxContext& context, size_t& pos);
    virtual ~Term() override = default;
};
*/

// Unary -> {PrefixOperator} Primary {PostfixOperator}
Unary::Unary(const locators::SpanLocator& pos) : ASTNode(pos) {}
optional<shared_ptr<Unary>> Unary::parse(SyntaxContext& context, size_t& pos) {
    size_t startpos = pos;
    vector<shared_ptr<PrefixOperator>> prefs;
    while (true) {
        auto op = PrefixOperator::parse(context, pos);
        if (!op.has_value()) break;
        prefs.push_back(*op);
    }
    auto prim = Primary::parse(context, pos);
    if (!prim.has_value()) {
        pos = startpos;
        return {};
    }
    vector<shared_ptr<PostfixOperator>> posts;
    while (true) {
        auto op = PostfixOperator::parse(context, pos);
        if (!op.has_value()) break;
        posts.push_back(*op);
    }
    auto res = make_shared<Unary>(context.MakeSpanFromTokens(startpos, pos));
    res->expr = *prim;
    res->prefixOps = prefs;
    res->postfixOps = posts;
    return res;
}
void Unary::AcceptVisitor(IASTVisitor& vis) {
    vis.VisitUnary(*this);
}

// Unary operator precedence:
// 1. function(args)  obj.field  arr[index]  // call & accessors
// 2. +num -num
// 3. obj is type
// 4. not value

// PrefixOperator -> tkNot | tkMinus | tkPlus
PrefixOperator::PrefixOperator(const locators::SpanLocator& pos) : ASTNode(pos) {}
int PrefixOperator::precedence() {  // the less, the more priority
    return precedence(kind);
}
int PrefixOperator::precedence(PrefixOperatorKind op) {
    switch (op) {
    case PrefixOperatorKind::Minus: case PrefixOperatorKind::Plus: return 2;
    case PrefixOperatorKind::Not: return 4;
    }
    return 0;
}
void PrefixOperator::AcceptVisitor(IASTVisitor& vis) {
    vis.VisitPrefixOperator(*this);
}
optional<shared_ptr<PrefixOperator>> PrefixOperator::parse(SyntaxContext& context, size_t& pos) {
    PrefixOperatorKind kind;
    if (AssertToken(context, pos, Token::Type::tkMinus))
        kind = PrefixOperatorKind::Minus;
    else if (AssertToken(context, pos, Token::Type::tkPlus))
        kind = PrefixOperatorKind::Plus;
    else if (AssertToken(context, pos, Token::Type::tkNot))
        kind = PrefixOperatorKind::Not;
    else return {};
    auto res = make_shared<PrefixOperator>(context.MakeSpanFromTokens(pos, pos + 1));
    res->kind = kind;
    ++pos;
    return res;
}

/*
// PostfixOperator -> TypecheckOperator | Call | AccessorOperator
class PostfixOperator : public ASTNode {
public:
    PostfixOperator(const locators::SpanLocator& pos);
    virtual int precedence() = 0;  // the less, the more priority
    static optional<shared_ptr<PostfixOperator>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~PostfixOperator() override = default;
};

// TypecheckOperator -> tkIs TypeId
class TypecheckOperator : public PostfixOperator {
public:
    TypecheckOperator(const locators::SpanLocator& pos);
    TypeId typeId;
    virtual int precedence() override;  // = 3
    static optional<shared_ptr<TypecheckOperator>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~TypecheckOperator() override = default;
};

// TypeId -> tkInt | tkReal | tkString | tkBool | tkNone | tkFunc
//     | tkOpenBracket tkClosedBracket | tkOpenCurlyBrace tkClosedCurlyBrace
optional<TypeId> parseTypeId(SyntaxContext& context, size_t& pos);

// Call -> tkOpenParenthesis < [ CommaExpressions ] > tkClosedParenthesis
class Call : public PostfixOperator {
public:
    Call(const locators::SpanLocator& pos);
    vector<shared_ptr<Expression>> args;
    virtual int precedence() override;  // = 1
    static optional<shared_ptr<Call>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~Call() override = default;
};

// AccessorOperator -> Accessor
class AccessorOperator : public PostfixOperator {
public:
    AccessorOperator(const locators::SpanLocator& pos);
    shared_ptr<Accessor> accessor;
    virtual int precedence() override;  // = 1
    static optional<shared_ptr<AccessorOperator>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~AccessorOperator() override = default;
};

// Primary -> PrimaryIdent | ParenthesesExpression | FuncLiteral | TokenLiteral | ArrayLiteral | TupleLiteral
class Primary : public ASTNode {
public:
    Primary(const locators::SpanLocator& pos);
    static optional<shared_ptr<Primary>> parse(SyntaxContext& context, size_t& pos);
    virtual ~Primary() override = default;
};

// PrimaryIdent -> tkIdent
class PrimaryIdent : public Primary {
public:
    PrimaryIdent(const locators::SpanLocator& pos);
    shared_ptr<IdentifierToken> name;
    static optional<shared_ptr<PrimaryIdent>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~PrimaryIdent() override = default;
};

// ParenthesesExpression -> tkOpenParenthesis < Expression > tkClosedParenthesis
class ParenthesesExpression : public Primary {
public:
    ParenthesesExpression(const locators::SpanLocator& pos);
    shared_ptr<Expression> expr;
    static optional<shared_ptr<ParenthesesExpression>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ParenthesesExpression() override = default;
};

// TupleLiteralElement -> [ tkIdent tkAssign ] Expression
class TupleLiteralElement : public ASTNode {
public:
    TupleLiteralElement(const locators::SpanLocator& pos);
    optional<shared_ptr<IdentifierToken>> ident;
    shared_ptr<Expression> expression;
    void AcceptVisitor(IASTVisitor& vis) override;
    optional<shared_ptr<TupleLiteralElement>> parse(SyntaxContext& context, size_t& pos);
    virtual ~TupleLiteralElement() override = default;
};

// TupleLiteral -> tkOpenCurlyBrace < [ TupleLiteralElement { tkComma TupleLiteralElement } ] > tkClosedCurlyBrace
class TupleLiteral : public Primary {
public:
    TupleLiteral(const locators::SpanLocator& pos);
    vector<shared_ptr<TupleLiteralElement>> elements;
    optional<shared_ptr<TupleLiteral>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~TupleLiteral() override = default;
};

// FuncBody -> ShortFuncBody | LongFuncBody
class FuncBody : public ASTNode {
public:
    FuncBody(const locators::SpanLocator& pos);
    static optional<shared_ptr<FuncBody>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~FuncBody() override = default;
};

// ShortFuncBody -> tkArrow Expression
class ShortFuncBody : public FuncBody {
public:
    ShortFuncBody(const locators::SpanLocator& pos);
    shared_ptr<Expression> expressionToReturn;
    static optional<shared_ptr<ShortFuncBody>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ShortFuncBody() override = default;
};

// LongFuncBody -> tkIs Body tkEnd
class LongFuncBody : public FuncBody {
public:
    LongFuncBody(const locators::SpanLocator& pos);
    shared_ptr<Body> funcBody;
    static optional<shared_ptr<LongFuncBody>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~LongFuncBody() override = default;
};

// FuncLiteral -> tkFunc tkOpenParenthesis < [ CommaIdents ] > tkClosedParenthesis FuncBody
class FuncLiteral : public Primary {
public:
    FuncLiteral(const locators::SpanLocator& pos);
    vector<shared_ptr<IdentifierToken>> parameters;
    optional<shared_ptr<FuncBody>> funcBody;
    optional<shared_ptr<FuncLiteral>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~FuncLiteral() override = default;
};

// TokenLiteral -> tkStringLiteral | tkIntLiteral | tkRealLiteral | tkTrue | tkFalse | tkNone
class TokenLiteral : public Primary {
public:
    TokenLiteral(const locators::SpanLocator& pos);
    TokenLiteralKind kind;
    // Yes, technically, `kind` is useless because `token->type` exists.
    // Still, it is nice when you have all the available options and nothing else.
    shared_ptr<Token> token;
    optional<shared_ptr<TokenLiteral>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~TokenLiteral() override = default;
};

// ArrayLiteral -> tkOpenBracket < [ CommaExpressions ] > tkClosedBracket
class ArrayLiteral : public Primary {
public:
    ArrayLiteral(const locators::SpanLocator& pos);
    optional<shared_ptr<Expression>> items;
    optional<shared_ptr<ArrayLiteral>> parse(SyntaxContext& context, size_t& pos);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ArrayLiteral() override = default;
};
*/
}

optional<shared_ptr<ast::Body>> analyze(const vector<shared_ptr<Token>>& tokens,
        const shared_ptr<const locators::CodeFile>& file, complog::ICompilationLog& log) {
    SyntaxContext context(tokens, file);
    size_t pos = 0;
    auto res = ast::parseProgram(context, pos);
    if (!res.has_value())
        for (auto& err : context.report.messages) log.Log(err);
    return res;
}
