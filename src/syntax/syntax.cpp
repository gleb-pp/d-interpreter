#include "syntax.h"

#include <cstdlib>
#include <memory>

#include "complog/CompilationLog.h"
#include "complog/CompilationMessage.h"
#include "lexer.h"
#include "locators/locator.h"
using namespace std;

// Auxiliary classes & functions

WrongNumberOfOperatorsSupplied::WrongNumberOfOperatorsSupplied(const std::string& astclass, size_t operandscount,
                                                               size_t operatorscount)
    : std::invalid_argument("\"" + astclass + "\"'s constructor received " + to_string(operandscount) +
                            " operands and " + to_string(operatorscount) + " operators.") {}

class EmptyVarStatement : public complog::CompilationMessage {
private:
    locators::Locator loc;

public:
    EmptyVarStatement(locators::Locator position)
        : CompilationMessage(complog::Severity::Error(), "EmptyVarStatement"), loc(position) {}
    void WriteMessageToStream(ostream& out, [[maybe_unused]] const FormatOptions& options) const override {
        out << "The \"var\" statement at " << loc.Pretty() << "must contain at least one declaration.\n";
    }
    vector<locators::Locator> Locators() const override { return {loc}; }
    virtual ~EmptyVarStatement() override = default;
};

class UnexpectedTokenTypeError : public complog::CompilationMessage {
private:
    locators::Locator loc;
    Token::Type expected, found;

public:
    UnexpectedTokenTypeError(locators::Locator position, Token::Type expected, Token::Type found)
        : CompilationMessage(complog::Severity::Error(), "UnexpectedTokenTypeError"),
          loc(position),
          expected(expected),
          found(found) {}
    void WriteMessageToStream(ostream& out, [[maybe_unused]] const FormatOptions& options) const override {
        out << "Unexpected token at " << loc.Pretty() << "; expected " << Token::TypeToString(expected)
            << ", but found " << Token::TypeToString(found) << ".\n";
    }
    vector<locators::Locator> Locators() const override { return {loc}; }
    virtual ~UnexpectedTokenTypeError() override = default;
};

// Use this to check if a token is of the provided type. This automatically
// produces a compilation log message if the token is unexpected
bool AssertToken(SyntaxContext& context, size_t pos, Token::Type expected) {
    auto& token = *context.tokens[pos];
    if (token.type == expected) return true;
    size_t textpos = token.span.position;
    context.report.Report(textpos,
                          make_shared<UnexpectedTokenTypeError>(context.MakeLocator(textpos), expected, token.type));
    return false;
}

locators::Locator SyntaxContext::MakeLocator(size_t pos) const { return locators::Locator(file, pos); }

locators::SpanLocator SyntaxContext::MakeSpanLocator(size_t position, size_t length) const {
    return {file, position, length};
}

locators::SpanLocator SyntaxContext::MakeSpanFromTokens(size_t firsttoken, size_t pastthelast) const {
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

SyntaxContext::SyntaxContext(const vector<shared_ptr<Token>>& tokens, const shared_ptr<const locators::CodeFile>& file,
                             complog::ICompilationLog& log)
    : tokens(tokens), compilationLog(&log), file(file) {}

void SyntaxErrorReport::Report(size_t pos, const std::shared_ptr<complog::CompilationMessage>& msg) {
    if (rightmostPos > pos) return;
    if (rightmostPos < pos) {
        rightmostPos = pos;
        messages.clear();
    }
    messages.push_back(msg);
}

// AST Node classes
// Every class in the ::parse implementation must:
// 1. On success, advance size_t& pos to the first unused token (after the last used token)
// 2. On failure, report unexpected tokens and what was expected
//    (perhaps by using AssertToken or context.report.Report)
// 3. On failure, reset size_t& pos to what it was at the start
// 4. Set SpanLocators (use context.MakeSpanFromTokens)
// If a class has an ::AcceptVisitor(vis) method, the implementation simply calls
//  vis.VisitMyClass(*this);

namespace ast {
#define VISITOR(classname) \
    void classname::AcceptVisitor(IASTVisitor& vis) { vis.Visit##classname(*this); }

ASTNode::ASTNode(const locators::SpanLocator& pos) : pos(pos) {}

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

Body::Body(const locators::SpanLocator& pos) : ASTNode(pos) {}
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
VISITOR(Body)

optional<shared_ptr<Body>> parseLoopBody(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkLoop)) return {};
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
    return EmptyStatement::parse(context, pos);
}

// VarStatement -> tkVar [tkNewLine] tkIdent [ AssignExpression ]
//     { tkComma [tkNewLine] tkIdent [ AssignExpression ] }
VarStatement::VarStatement(const locators::SpanLocator& pos) : Statement(pos) {}
optional<shared_ptr<VarStatement>> VarStatement::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkVar)) return {};
    size_t startpos = pos++;
    pos += AssertToken(context, pos, Token::Type::tkNewLine);
    bool first = true;
    vector<pair<string, optional<shared_ptr<Expression>>>> defs;
    while (true) {
        size_t prevpos = pos;
        if (!first) {
            if (!AssertToken(context, pos, Token::Type::tkComma)) break;
            ++pos;
            pos += AssertToken(context, pos, Token::Type::tkNewLine);
        }
        first = false;
        if (!AssertToken(context, pos, Token::Type::tkIdent)) {
            pos = prevpos;
            break;
        }
        auto tkIdent = dynamic_pointer_cast<IdentifierToken>(context.tokens[pos++]);
        auto optAsg = parseAssignExpression(context, pos);
        defs.emplace_back(tkIdent->identifier, optAsg);
    }
    if (defs.empty()) {
        context.compilationLog->Log(
            make_shared<EmptyVarStatement>(context.MakeLocator(context.tokens[startpos]->span.position)));
        return {};
    }
    auto res = make_shared<VarStatement>(context.MakeSpanFromTokens(startpos, pos));
    res->definitions = defs;
    return res;
}
VISITOR(VarStatement)

// IfStatement -> tkIf < Expression > tkThen [tkNewLine] Body
//     [ tkElse [tkNewLine] Body ] tkEnd
IfStatement::IfStatement(const locators::SpanLocator& pos, const shared_ptr<Expression>& condition,
                         const shared_ptr<Body>& doIfTrue, const optional<shared_ptr<Body>>& doIfFalse)
    : Statement(pos), condition(condition), doIfTrue(doIfTrue), doIfFalse(doIfFalse) {}
optional<shared_ptr<IfStatement>> IfStatement::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkIf)) return {};
    size_t startpos = pos++;
    auto optExpr = Expression::parse(context, pos);
    if (!optExpr.has_value()) {
        pos = startpos;
        return {};
    }
    if (!AssertToken(context, pos, Token::Type::tkThen)) {
        pos = startpos;
        return {};
    }
    ++pos;
    pos += AssertToken(context, pos, Token::Type::tkNewLine);
    auto optDoTrue = Body::parse(context, pos);
    if (!optDoTrue.has_value()) {
        pos = startpos;
        return {};
    }
    optional<shared_ptr<Body>> optDoFalse;
    {
        size_t prevpos = pos;
        if (AssertToken(context, pos, Token::Type::tkElse)) {
            ++pos;
            pos += AssertToken(context, pos, Token::Type::tkNewLine);
            optDoFalse = Body::parse(context, pos);
            if (!optDoFalse.has_value()) pos = prevpos;
        }
    }
    if (!AssertToken(context, pos, Token::Type::tkEnd)) {
        pos = startpos;
        return {};
    }
    ++pos;
    return make_shared<IfStatement>(context.MakeSpanFromTokens(startpos, pos), *optExpr, *optDoTrue, optDoFalse);
}
VISITOR(IfStatement)

// ShortIfStatement -> tkIf < Expression > tkArrow Statement
ShortIfStatement::ShortIfStatement(const locators::SpanLocator& pos, const shared_ptr<Expression>& condition,
                                   const shared_ptr<Statement>& doIfTrue)
    : Statement(pos), condition(condition), doIfTrue(doIfTrue) {}
optional<shared_ptr<ShortIfStatement>> ShortIfStatement::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkIf)) return {};
    size_t startpos = pos++;
    auto optExpr = Expression::parse(context, pos);
    if (!optExpr.has_value()) {
        pos = startpos;
        return {};
    }
    if (!AssertToken(context, pos, Token::Type::tkArrow)) {
        pos = startpos;
        return {};
    }
    ++pos;
    auto optStatement = Statement::parse(context, pos);
    if (!optStatement.has_value()) {
        pos = startpos;
        return {};
    }
    return make_shared<ShortIfStatement>(context.MakeSpanFromTokens(startpos, pos), *optExpr, *optStatement);
}
VISITOR(ShortIfStatement)

// WhileStatement -> tkWhile < Expression > LoopBody
WhileStatement::WhileStatement(const locators::SpanLocator& pos, const shared_ptr<Expression>& condition,
                               const shared_ptr<Body>& action)
    : Statement(pos), condition(condition), action(action) {}
optional<shared_ptr<WhileStatement>> WhileStatement::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkWhile)) return {};
    size_t startpos = pos;
    auto optExpr = Expression::parse(context, pos);
    if (!optExpr.has_value()) {
        pos = startpos;
        return {};
    }
    auto optBody = parseLoopBody(context, pos);
    if (!optBody.has_value()) {
        pos = startpos;
        return {};
    }
    return make_shared<WhileStatement>(context.MakeSpanFromTokens(startpos, pos), *optExpr, *optBody);
}
VISITOR(WhileStatement)

// ForStatement -> tkFor [ tkIdent tkIn ] < Expression > [ tkRange < Expression > ] [tkNewLine] LoopBody
ForStatement::ForStatement(const locators::SpanLocator& pos) : Statement(pos) {}
// optional<shared_ptr<IdentifierToken>> optVariableName;
// shared_ptr<Expression> startOrList;
// optional<shared_ptr<Expression>> end;
// shared_ptr<Body> action;
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
VISITOR(ForStatement)

// LoopStatement -> LoopBody
LoopStatement::LoopStatement(const locators::SpanLocator& pos, const shared_ptr<Body>& body)
    : Statement(pos), body(body) {}
optional<shared_ptr<LoopStatement>> LoopStatement::parse(SyntaxContext& context, size_t& pos) {
    size_t startpos = pos;
    auto body = parseLoopBody(context, pos);
    if (!body.has_value()) return {};
    return make_shared<LoopStatement>(context.MakeSpanFromTokens(startpos, pos), *body);
}
VISITOR(LoopStatement)

// ExitStatement -> tkExit
ExitStatement::ExitStatement(const locators::SpanLocator& pos) : Statement(pos) {}
optional<shared_ptr<ExitStatement>> ExitStatement::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkExit)) return {};
    ++pos;
    return make_shared<ExitStatement>(context.MakeSpanFromTokens(pos - 1, pos));
}
VISITOR(ExitStatement)

// AssignStatement -> Reference tkAssign Expression
AssignStatement::AssignStatement(const locators::SpanLocator& pos, const shared_ptr<Reference>& dest,
                                 const shared_ptr<Expression>& src)
    : Statement(pos), dest(dest), src(src) {}
optional<shared_ptr<AssignStatement>> AssignStatement::parse(SyntaxContext& context, size_t& pos) {
    const size_t startpos = pos;
    auto optRef = Reference::parse(context, pos);
    if (!optRef.has_value()) return {};
    if (!AssertToken(context, pos, Token::Type::tkAssign)) {
        pos = startpos;
        return {};
    }
    ++pos;
    auto optExpr = Expression::parse(context, pos);
    if (!optExpr.has_value()) {
        pos = startpos;
        return {};
    }
    return make_shared<AssignStatement>(context.MakeSpanFromTokens(startpos, pos), *optRef, *optExpr);
}
VISITOR(AssignStatement)

// PrintStatement -> tkPrint [ CommaExpressions ]
PrintStatement::PrintStatement(const locators::SpanLocator& pos, const vector<shared_ptr<Expression>>& expressions)
    : Statement(pos), expressions(expressions) {}
optional<shared_ptr<PrintStatement>> PrintStatement::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkPrint)) return {};
    const size_t startpos = pos++;
    auto exprs = CommaExpressions::parse(context, pos);
    vector<shared_ptr<Expression>> lst;
    if (exprs.has_value()) lst = exprs.value()->expressions;
    return make_shared<PrintStatement>(context.MakeSpanFromTokens(startpos, pos), lst);
}
VISITOR(PrintStatement)

// ReturnStatement -> tkReturn [ Expression ]
ReturnStatement::ReturnStatement(const locators::SpanLocator& pos, const optional<shared_ptr<Expression>>& returnValue)
    : Statement(pos), returnValue(returnValue) {}
optional<shared_ptr<ReturnStatement>> ReturnStatement::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkReturn)) return {};
    const size_t startpos = pos++;
    auto optExpr = Expression::parse(context, pos);
    return make_shared<ReturnStatement>(context.MakeSpanFromTokens(startpos, pos), optExpr);
}
VISITOR(ReturnStatement)

// ExpressionStatement -> Expression
ExpressionStatement::ExpressionStatement(const locators::SpanLocator& pos, const shared_ptr<Expression>& expr)
    : Statement(pos), expr(expr) {}
optional<shared_ptr<ExpressionStatement>> ExpressionStatement::parse(SyntaxContext& context, size_t& pos) {
    auto res = Expression::parse(context, pos);
    if (!res.has_value()) return {};
    return make_shared<ExpressionStatement>(res.value()->pos, res.value());
}
VISITOR(ExpressionStatement)

// EmptyStatement ->
EmptyStatement::EmptyStatement(const locators::SpanLocator& pos) : Statement(pos) {}
// parse does nothing and always succeeds
shared_ptr<EmptyStatement> EmptyStatement::parse(const SyntaxContext& context, size_t pos) {
    return make_shared<EmptyStatement>(context.MakeSpanFromTokens(pos, pos));
}
VISITOR(EmptyStatement)

// CommaExpressions -> Expression { tkComma Expression }
CommaExpressions::CommaExpressions(const locators::SpanLocator& pos, const vector<shared_ptr<Expression>>& expressions)
    : ASTNode(pos), expressions(expressions) {}
optional<shared_ptr<CommaExpressions>> CommaExpressions::parse(SyntaxContext& context, size_t& pos) {
    const size_t startpos = pos;
    bool first = true;
    vector<shared_ptr<Expression>> exprs;
    while (true) {
        const size_t prevpos = pos;
        if (!first) {
            if (!AssertToken(context, pos, Token::Type::tkComma)) break;
            ++pos;
        }
        first = false;
        auto optExpr = Expression::parse(context, pos);
        if (!optExpr.has_value()) {
            pos = prevpos;
            break;
        }
        exprs.push_back(*optExpr);
    }
    if (exprs.empty()) return {};
    return make_shared<CommaExpressions>(context.MakeSpanFromTokens(startpos, pos), exprs);
}
VISITOR(CommaExpressions)

// CommaIdents -> tkIdent { tkComma tkIdent }
CommaIdents::CommaIdents(const locators::SpanLocator& pos, const vector<shared_ptr<IdentifierToken>>& idents)
    : ASTNode(pos), idents(idents) {}
optional<shared_ptr<CommaIdents>> CommaIdents::parse(SyntaxContext& context, size_t& pos) {
    const size_t startpos = pos;
    bool first = true;
    vector<shared_ptr<IdentifierToken>> tokens;
    while (true) {
        const size_t prevpos = pos;
        if (!first) {
            if (!AssertToken(context, pos, Token::Type::tkComma)) break;
            ++pos;
        }
        first = false;
        if (!AssertToken(context, pos, Token::Type::tkIdent)) {
            pos = prevpos;
            break;
        }
        auto ident = dynamic_pointer_cast<IdentifierToken>(context.tokens[pos++]);
        tokens.push_back(ident);
    }
    if (tokens.empty()) return {};
    return make_shared<CommaIdents>(context.MakeSpanFromTokens(startpos, pos), tokens);
}
VISITOR(CommaIdents)

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

IdentMemberAccessor::IdentMemberAccessor(const locators::SpanLocator& pos, const shared_ptr<IdentifierToken>& name)
    : Accessor(pos), name(name) {}
optional<shared_ptr<IdentMemberAccessor>> IdentMemberAccessor::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkDot)) return {};
    size_t startpos = pos;
    ++pos;
    if (!AssertToken(context, pos, Token::Type::tkIdent)) {
        pos = startpos;
        return {};
    }
    auto tk = dynamic_pointer_cast<IdentifierToken>(context.tokens[pos++]);
    return make_shared<IdentMemberAccessor>(context.MakeSpanFromTokens(startpos, pos), tk);
}
VISITOR(IdentMemberAccessor)

IntLiteralMemberAccessor::IntLiteralMemberAccessor(const locators::SpanLocator& pos,
                                                   const shared_ptr<IntegerToken>& index)
    : Accessor(pos), index(index) {}
optional<shared_ptr<IntLiteralMemberAccessor>> IntLiteralMemberAccessor::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkDot)) return {};
    size_t startpos = pos;
    ++pos;
    if (!AssertToken(context, pos, Token::Type::tkIntLiteral)) {
        pos = startpos;
        return {};
    }
    auto tk = dynamic_pointer_cast<IntegerToken>(context.tokens[pos++]);
    return make_shared<IntLiteralMemberAccessor>(context.MakeSpanFromTokens(startpos, pos), tk);
}
VISITOR(IntLiteralMemberAccessor)

ParenMemberAccessor::ParenMemberAccessor(const locators::SpanLocator& pos, const shared_ptr<Expression>& expr)
    : Accessor(pos), expr(expr) {}
optional<shared_ptr<ParenMemberAccessor>> ParenMemberAccessor::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkDot)) return {};
    size_t startpos = pos;
    ++pos;
    auto expr = ParenthesesExpression::parse(context, pos);
    if (!expr.has_value()) {
        pos = startpos;
        return {};
    }
    return make_shared<ParenMemberAccessor>(context.MakeSpanFromTokens(startpos, pos), expr.value()->expr);
}
VISITOR(ParenMemberAccessor)

// IndexAccessor -> tkOpenBracket < Expression > tkClosedBracket
IndexAccessor::IndexAccessor(const locators::SpanLocator& pos, const shared_ptr<Expression>& expressionInBrackets)
    : Accessor(pos), expressionInBrackets(expressionInBrackets) {}
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
    return make_shared<IndexAccessor>(context.MakeSpanFromTokens(startpos, pos), *expr);
}
VISITOR(IndexAccessor)

// Reference -> tkIdent { Accessor }
Reference::Reference(const locators::SpanLocator& pos, const string& baseIdent,
                     const vector<shared_ptr<Accessor>>& accessorChain)
    : ASTNode(pos), baseIdent(baseIdent), accessorChain(accessorChain) {}
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
    return make_shared<Reference>(context.MakeSpanFromTokens(startpos, pos), ident->identifier, chain);
}
VISITOR(Reference)

// Expression -> OrOperator { tkXor OrOperator }
Expression::Expression(const locators::SpanLocator& pos, const vector<shared_ptr<OrOperator>>& operands)
    : ASTNode(pos), operands(operands) {}
optional<shared_ptr<Expression>> Expression::parse(SyntaxContext& context, size_t& pos) {
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
    return make_shared<Expression>(context.MakeSpanFromTokens(startpos, pos), operands);
}
VISITOR(Expression)

// OrOperator -> AndOperator { tkOr AndOperator }
OrOperator::OrOperator(const locators::SpanLocator& pos, const vector<shared_ptr<AndOperator>>& operands)
    : ASTNode(pos), operands(operands) {}
optional<shared_ptr<OrOperator>> OrOperator::parse(SyntaxContext& context, size_t& pos) {
    const size_t startpos = pos;
    vector<shared_ptr<AndOperator>> ops;
    bool first = true;
    while (true) {
        const size_t prevpos = pos;
        if (!first) {
            if (!AssertToken(context, pos, Token::Type::tkOr)) break;
            ++pos;
        }
        first = false;
        auto optAnd = AndOperator::parse(context, pos);
        if (!optAnd.has_value()) {
            pos = prevpos;
            break;
        }
        ops.push_back(*optAnd);
    }
    if (ops.empty()) return {};
    return make_shared<OrOperator>(context.MakeSpanFromTokens(startpos, pos), ops);
}
VISITOR(OrOperator)

// AndOperator -> BinaryRelation { tkAnd BinaryRelation }
AndOperator::AndOperator(const locators::SpanLocator& pos, const vector<shared_ptr<BinaryRelation>>& operands)
    : ASTNode(pos), operands(operands) {}
optional<shared_ptr<AndOperator>> AndOperator::parse(SyntaxContext& context, size_t& pos) {
    const size_t startpos = pos;
    vector<shared_ptr<BinaryRelation>> ops;
    bool first = true;
    while (true) {
        const size_t prevpos = pos;
        if (!first) {
            if (!AssertToken(context, pos, Token::Type::tkOr)) break;
            ++pos;
        }
        first = false;
        auto optBinRel = BinaryRelation::parse(context, pos);
        if (!optBinRel.has_value()) {
            pos = prevpos;
            break;
        }
        ops.push_back(*optBinRel);
    }
    if (ops.empty()) return {};
    return make_shared<AndOperator>(context.MakeSpanFromTokens(startpos, pos), ops);
}
VISITOR(AndOperator)

// BinaryRelation -> Sum { BinaryRelationOperator Sum }
BinaryRelation::BinaryRelation(const locators::SpanLocator& pos, const vector<shared_ptr<Sum>>& operands,
                               const vector<BinaryRelationOperator>& operators)
    : ASTNode(pos), operands(operands), operators(operators) {
    size_t nds = operands.size(), ors = operators.size();
    if (nds != ors + 1) throw WrongNumberOfOperatorsSupplied("BinaryRelation", nds, ors);
}
optional<shared_ptr<BinaryRelation>> BinaryRelation::parse(SyntaxContext& context, size_t& pos) {
    const size_t startpos = pos;
    vector<shared_ptr<Sum>> nds;
    vector<BinaryRelationOperator> ors;
    bool first = true;
    while (true) {
        const size_t prevpos = pos;
        if (!first) {
            auto binOperator = parseBinaryRelationOperator(context, pos);
            if (!binOperator.has_value()) break;
            ors.push_back(*binOperator);
        }
        first = false;
        auto optSum = Sum::parse(context, pos);
        if (!optSum.has_value()) {
            pos = prevpos;
            break;
        }
        nds.push_back(*optSum);
    }
    if (nds.empty()) return {};
    return make_shared<BinaryRelation>(context.MakeSpanFromTokens(startpos, pos), nds, ors);
}
VISITOR(BinaryRelation)

// BinaryRelationOperator -> tkLess | tkLessEq | tkGreater | tkGreaterEq | tkEqual | tkNotEqual
optional<BinaryRelationOperator> parseBinaryRelationOperator(SyntaxContext& context, size_t& pos) {
#define TRY(tk, op)                                   \
    if (AssertToken(context, pos, Token::Type::tk)) { \
        ++pos;                                        \
        return BinaryRelationOperator::op;            \
    }
    TRY(tkLess, Less)
    TRY(tkLessEq, LessEq)
    TRY(tkGreater, Greater)
    TRY(tkGreaterEq, GreaterEq)
    TRY(tkEqual, Equal)
    TRY(tkNotEqual, NotEqual)
    return {};
#undef TRY
}

// Sum -> Term { (tkPlus | tkMinus) Term }
Sum::Sum(const locators::SpanLocator& pos, const vector<shared_ptr<Term>>& terms, const vector<SumOperator>& operators)
    : ASTNode(pos), terms(terms), operators(operators) {}
optional<shared_ptr<Sum>> Sum::parse(SyntaxContext& context, size_t& pos) {
    const size_t startpos = pos;
    vector<shared_ptr<Term>> nds;
    vector<SumOperator> ors;
    bool first = true;
    while (true) {
        const size_t prevpos = pos;
        if (!first) {
            if (AssertToken(context, pos, Token::Type::tkPlus)) {
                ++pos;
                ors.push_back(SumOperator::Plus);
            } else if (AssertToken(context, pos, Token::Type::tkMinus)) {
                ++pos;
                ors.push_back(SumOperator::Minus);
            } else
                break;
        }
        first = false;
        auto optTerm = Term::parse(context, pos);
        if (!optTerm.has_value()) {
            pos = prevpos;
            break;
        }
        nds.push_back(*optTerm);
    }
    if (nds.empty()) return {};
    return make_shared<Sum>(context.MakeSpanFromTokens(startpos, pos), nds, ors);
}
VISITOR(Sum)

// Term -> Unary { (tkTimes | tkDivide) Unary }
Term::Term(const locators::SpanLocator& pos, const vector<shared_ptr<Unary>>& unaries,
           const vector<TermOperator>& operators)
    : ASTNode(pos), unaries(unaries), operators(operators) {}
optional<shared_ptr<Term>> Term::parse(SyntaxContext& context, size_t& pos) {
    const size_t startpos = pos;
    vector<shared_ptr<Unary>> nds;
    vector<TermOperator> ors;
    bool first = true;
    while (true) {
        const size_t prevpos = pos;
        if (!first) {
            if (AssertToken(context, pos, Token::Type::tkTimes)) {
                ++pos;
                ors.push_back(TermOperator::Times);
            } else if (AssertToken(context, pos, Token::Type::tkDivide)) {
                ++pos;
                ors.push_back(TermOperator::Divide);
            } else
                break;
        }
        first = false;
        auto optUnary = Unary::parse(context, pos);
        if (!optUnary.has_value()) {
            pos = prevpos;
            break;
        }
        nds.push_back(*optUnary);
    }
    if (nds.empty()) return {};
    return make_shared<Term>(context.MakeSpanFromTokens(startpos, pos), nds, ors);
}
VISITOR(Term)

// Unary -> {PrefixOperator} Primary {PostfixOperator}
Unary::Unary(const locators::SpanLocator& pos, const vector<shared_ptr<PrefixOperator>>& prefixOps,
             const vector<shared_ptr<PostfixOperator>>& postfixOps, const shared_ptr<Primary>& expr)
    : ASTNode(pos), prefixOps(prefixOps), postfixOps(postfixOps), expr(expr) {}
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
    auto res = make_shared<Unary>(context.MakeSpanFromTokens(startpos, pos), prefs, posts, *prim);
    return res;
}
VISITOR(Unary)

// Unary operator precedence:
// 1. function(args)  obj.field  arr[index]  // call & accessors
// 2. +num -num
// 3. obj is type
// 4. not value

// PrefixOperator -> tkNot | tkMinus | tkPlus
PrefixOperator::PrefixOperator(const locators::SpanLocator& pos, PrefixOperatorKind kind) : ASTNode(pos), kind(kind) {}
int PrefixOperator::precedence() {  // the less, the more priority
    return precedence(kind);
}
int PrefixOperator::precedence(PrefixOperatorKind op) {
    switch (op) {
        case PrefixOperatorKind::Minus:
        case PrefixOperatorKind::Plus:
            return 2;
        case PrefixOperatorKind::Not:
            return 4;
    }
    return 0;
}
VISITOR(PrefixOperator)
optional<shared_ptr<PrefixOperator>> PrefixOperator::parse(SyntaxContext& context, size_t& pos) {
    PrefixOperatorKind kind;
    if (AssertToken(context, pos, Token::Type::tkMinus))
        kind = PrefixOperatorKind::Minus;
    else if (AssertToken(context, pos, Token::Type::tkPlus))
        kind = PrefixOperatorKind::Plus;
    else if (AssertToken(context, pos, Token::Type::tkNot))
        kind = PrefixOperatorKind::Not;
    else
        return {};
    auto res = make_shared<PrefixOperator>(context.MakeSpanFromTokens(pos, pos + 1), kind);
    ++pos;
    return res;
}

// PostfixOperator -> TypecheckOperator | Call | AccessorOperator
PostfixOperator::PostfixOperator(const locators::SpanLocator& pos) : ASTNode(pos) {}
optional<shared_ptr<PostfixOperator>> PostfixOperator::parse(SyntaxContext& context, size_t& pos) {
#define TRY(classname)                             \
    {                                              \
        auto res = classname::parse(context, pos); \
        if (res.has_value()) return res;           \
    }
    TRY(TypecheckOperator)
    TRY(Call)
    TRY(AccessorOperator)
    return {};
#undef TRY
}

// TypecheckOperator -> tkIs TypeId
TypecheckOperator::TypecheckOperator(const locators::SpanLocator& pos, TypeId typeId)
    : PostfixOperator(pos), typeId(typeId) {}
int TypecheckOperator::precedence() { return 3; }
optional<shared_ptr<TypecheckOperator>> TypecheckOperator::parse(SyntaxContext& context, size_t& pos) {
    const size_t startpos = pos;
    if (!AssertToken(context, pos, Token::Type::tkIs)) return {};
    ++pos;
    auto op = parseTypeId(context, pos);
    if (!op.has_value()) {
        pos = startpos;
        return {};
    }
    return make_shared<TypecheckOperator>(context.MakeSpanFromTokens(startpos, pos), *op);
}
VISITOR(TypecheckOperator)

// TypeId -> tkInt | tkReal | tkString | tkBool | tkNone | tkFunc
//     | tkOpenBracket tkClosedBracket | tkOpenCurlyBrace tkClosedCurlyBrace
optional<TypeId> parseTypeId(SyntaxContext& context, size_t& pos) {
#define TRY(name)                                           \
    if (AssertToken(context, pos, Token::Type::tk##name)) { \
        ++pos;                                              \
        return TypeId::name;                                \
    }
    TRY(Int)
    TRY(Real)
    TRY(String)
    TRY(Bool)
    TRY(None)
    TRY(Func)
#undef TRY
    if (AssertToken(context, pos, Token::Type::tkOpenBracket)) {
        ++pos;
        if (AssertToken(context, pos, Token::Type::tkClosedBracket)) {
            ++pos;
            return TypeId::List;
        }
        --pos;
    }
    if (AssertToken(context, pos, Token::Type::tkOpenCurlyBrace)) {
        ++pos;
        if (AssertToken(context, pos, Token::Type::tkClosedCurlyBrace)) {
            ++pos;
            return TypeId::Tuple;
        }
        --pos;
    }
    return {};
}

// Call -> tkOpenParenthesis < [ CommaExpressions ] > tkClosedParenthesis
Call::Call(const locators::SpanLocator& pos, const vector<shared_ptr<Expression>>& args)
    : PostfixOperator(pos), args(args) {}
int Call::precedence() { return 1; }
optional<shared_ptr<Call>> Call::parse(SyntaxContext& context, size_t& pos) {
    const size_t startpos = pos;
    if (!AssertToken(context, pos, Token::Type::tkOpenParenthesis)) return {};
    auto comexpr = CommaExpressions::parse(context, pos);
    vector<shared_ptr<Expression>> args;
    if (comexpr.has_value()) args = comexpr.value()->expressions;
    if (!AssertToken(context, pos, Token::Type::tkClosedParenthesis)) {
        pos = startpos;
        return {};
    }
    ++pos;
    return make_shared<Call>(context.MakeSpanFromTokens(startpos, pos), args);
}
VISITOR(Call)

// AccessorOperator -> Accessor
AccessorOperator::AccessorOperator(const locators::SpanLocator& pos, const shared_ptr<Accessor>& accessor)
    : PostfixOperator(pos), accessor(accessor) {}
int AccessorOperator::precedence() { return 1; }
optional<shared_ptr<AccessorOperator>> AccessorOperator::parse(SyntaxContext& context, size_t& pos) {
    auto res = Accessor::parse(context, pos);
    if (!res.has_value()) return {};
    return make_shared<AccessorOperator>(res.value()->pos, res.value());
}
VISITOR(AccessorOperator)

// Primary -> PrimaryIdent | ParenthesesExpression | FuncLiteral | TokenLiteral | ArrayLiteral | TupleLiteral
Primary::Primary(const locators::SpanLocator& pos) : ASTNode(pos) {}
optional<shared_ptr<Primary>> Primary::parse(SyntaxContext& context, size_t& pos) {
#define TRY(classname)                             \
    {                                              \
        auto res = classname::parse(context, pos); \
        if (res.has_value()) return res;           \
    }
    TRY(PrimaryIdent)
    TRY(ParenthesesExpression)
    TRY(FuncLiteral)
    TRY(TokenLiteral)
    TRY(ArrayLiteral)
    TRY(TupleLiteral)
#undef TRY
    return {};
}

// PrimaryIdent -> tkIdent
PrimaryIdent::PrimaryIdent(const locators::SpanLocator& pos, const shared_ptr<IdentifierToken>& name)
    : Primary(pos), name(name) {}
optional<shared_ptr<PrimaryIdent>> PrimaryIdent::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkIdent)) return {};
    auto res = make_shared<PrimaryIdent>(context.MakeSpanFromTokens(pos, pos + 1),
                                         dynamic_pointer_cast<IdentifierToken>(context.tokens[pos]));
    ++pos;
    return res;
}
VISITOR(PrimaryIdent)

// ParenthesesExpression -> tkOpenParenthesis < Expression > tkClosedParenthesis
ParenthesesExpression::ParenthesesExpression(const locators::SpanLocator& pos, const shared_ptr<Expression>& expr)
    : Primary(pos), expr(expr) {}
optional<shared_ptr<ParenthesesExpression>> ParenthesesExpression::parse(SyntaxContext& context, size_t& pos) {
    const size_t startpos = pos;
    if (!AssertToken(context, pos, Token::Type::tkOpenParenthesis)) return {};
    ++pos;
    auto expr = Expression::parse(context, pos);
    if (!expr.has_value()) {
        pos = startpos;
        return {};
    }
    if (!AssertToken(context, pos, Token::Type::tkClosedParenthesis)) {
        pos = startpos;
        return {};
    }
    ++pos;
    return make_shared<ParenthesesExpression>(context.MakeSpanFromTokens(startpos, pos), *expr);
}
VISITOR(ParenthesesExpression)

// TupleLiteralElement -> [ tkIdent tkAssign ] Expression
TupleLiteralElement::TupleLiteralElement(const locators::SpanLocator& pos,
                                         const optional<shared_ptr<IdentifierToken>>& ident,
                                         const shared_ptr<Expression>& expression)
    : ASTNode(pos), ident(ident), expression(expression) {}
optional<shared_ptr<TupleLiteralElement>> TupleLiteralElement::parse(SyntaxContext& context, size_t& pos) {
    const size_t startpos = pos;
    optional<shared_ptr<IdentifierToken>> ident;
    if (AssertToken(context, pos, Token::Type::tkIdent)) {
        ++pos;
        if (AssertToken(context, pos, Token::Type::tkAssign)) {
            ident = dynamic_pointer_cast<IdentifierToken>(context.tokens[pos - 1]);
            ++pos;
        } else
            pos = startpos;
    }
    auto optExpr = Expression::parse(context, pos);
    if (!optExpr.has_value()) {
        pos = startpos;
        return {};
    }
    return make_shared<TupleLiteralElement>(context.MakeSpanFromTokens(startpos, pos), ident, *optExpr);
}
VISITOR(TupleLiteralElement)

// TupleLiteral -> tkOpenCurlyBrace < [ TupleLiteralElement { tkComma TupleLiteralElement } ] > tkClosedCurlyBrace
TupleLiteral::TupleLiteral(const locators::SpanLocator& pos, const vector<shared_ptr<TupleLiteralElement>>& elements)
    : Primary(pos), elements(elements) {}
optional<shared_ptr<TupleLiteral>> TupleLiteral::parse(SyntaxContext& context, size_t& pos) {
    const size_t startpos = pos;
    if (!AssertToken(context, pos, Token::Type::tkOpenCurlyBrace)) return {};
    ++pos;
    vector<shared_ptr<TupleLiteralElement>> elems;
    bool first = true;
    while (true) {
        const size_t prevpos = pos;
        if (!first) {
            if (!AssertToken(context, pos, Token::Type::tkComma)) break;
            ++pos;
        }
        first = false;
        auto elem = TupleLiteralElement::parse(context, pos);
        if (!elem.has_value()) {
            pos = prevpos;
            break;
        }
        elems.push_back(*elem);
    }
    if (!AssertToken(context, pos, Token::Type::tkClosedCurlyBrace)) {
        pos = startpos;
        return {};
    }
    ++pos;
    return make_shared<TupleLiteral>(context.MakeSpanFromTokens(startpos, pos), elems);
}
VISITOR(TupleLiteral)

// FuncBody -> ShortFuncBody | LongFuncBody
FuncBody::FuncBody(const locators::SpanLocator& pos) : ASTNode(pos) {}
optional<shared_ptr<FuncBody>> FuncBody::parse(SyntaxContext& context, size_t& pos) {
#define TRY(classname)                             \
    {                                              \
        auto res = classname::parse(context, pos); \
        if (res.has_value()) return res;           \
    }
    TRY(ShortFuncBody)
    TRY(LongFuncBody)
#undef TRY
    return {};
}

// ShortFuncBody -> tkArrow Expression
ShortFuncBody::ShortFuncBody(const locators::SpanLocator& pos, const shared_ptr<Expression>& expressionToReturn)
    : FuncBody(pos), expressionToReturn(expressionToReturn) {}
optional<shared_ptr<ShortFuncBody>> ShortFuncBody::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkArrow)) return {};
    const size_t startpos = pos++;
    auto optExpr = Expression::parse(context, pos);
    if (!optExpr.has_value()) {
        pos = startpos;
        return {};
    }
    return make_shared<ShortFuncBody>(context.MakeSpanFromTokens(startpos, pos), *optExpr);
}
VISITOR(ShortFuncBody)

// LongFuncBody -> tkIs Body tkEnd
LongFuncBody::LongFuncBody(const locators::SpanLocator& pos, const shared_ptr<Body>& funcBody)
    : FuncBody(pos), funcBody(funcBody) {}
optional<shared_ptr<LongFuncBody>> LongFuncBody::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkIs)) return {};
    const size_t startpos = pos++;
    auto optBody = Body::parse(context, pos);
    if (!optBody.has_value()) {
        pos = startpos;
        return {};
    }
    if (!AssertToken(context, pos, Token::Type::tkEnd)) {
        pos = startpos;
        return {};
    }
    ++pos;
    return make_shared<LongFuncBody>(context.MakeSpanFromTokens(startpos, pos), *optBody);
}
VISITOR(LongFuncBody)

// FuncLiteral -> tkFunc tkOpenParenthesis < [ CommaIdents ] > tkClosedParenthesis FuncBody
FuncLiteral::FuncLiteral(const locators::SpanLocator& pos, const vector<shared_ptr<IdentifierToken>>& parameters,
                         const optional<shared_ptr<FuncBody>>& funcBody)
    : Primary(pos), parameters(parameters), funcBody(funcBody) {}
optional<shared_ptr<FuncLiteral>> FuncLiteral::parse(SyntaxContext& context, size_t& pos) {
    const size_t startpos = pos;
    if (!AssertToken(context, pos, Token::Type::tkFunc)) return {};
    ++pos;
    if (!AssertToken(context, pos, Token::Type::tkOpenParenthesis)) {
        pos = startpos;
        return {};
    }
    ++pos;
    vector<shared_ptr<IdentifierToken>> params;
    {
        auto optIdents = CommaIdents::parse(context, pos);
        if (optIdents.has_value()) params = optIdents.value()->idents;
    }
    if (!AssertToken(context, pos, Token::Type::tkClosedParenthesis)) {
        pos = startpos;
        return {};
    }
    ++pos;
    auto body = FuncBody::parse(context, pos);
    if (!body.has_value()) {
        pos = startpos;
        return {};
    }
    return make_shared<FuncLiteral>(context.MakeSpanFromTokens(startpos, pos), params, *body);
}
VISITOR(FuncLiteral)

// TokenLiteral -> tkStringLiteral | tkIntLiteral | tkRealLiteral | tkTrue | tkFalse | tkNone
TokenLiteral::TokenLiteral(const locators::SpanLocator& pos, TokenLiteralKind kind, const shared_ptr<Token>& token)
    : Primary(pos), kind(kind), token(token) {}
optional<shared_ptr<TokenLiteral>> TokenLiteral::parse(SyntaxContext& context, size_t& pos) {
#define TRY(tktype, reskind)                                  \
    if (AssertToken(context, pos, Token::Type::tk##tktype)) { \
        kind = TokenLiteralKind::reskind;                     \
        token = context.tokens[pos++];                        \
    } else
    TokenLiteralKind kind;
    shared_ptr<Token> token;
    TRY(StringLiteral, String)
    TRY(IntLiteral, Int)
    TRY(RealLiteral, Real)
    TRY(True, True)
    TRY(False, False)
    TRY(None, None)
    return {};
#undef TRY
    return make_shared<TokenLiteral>(context.MakeSpanFromTokens(pos - 1, pos), kind, token);
}
VISITOR(TokenLiteral)

// ArrayLiteral -> tkOpenBracket < [ CommaExpressions ] > tkClosedBracket
ArrayLiteral::ArrayLiteral(const locators::SpanLocator& pos, const vector<shared_ptr<Expression>>& items)
    : Primary(pos), items(items) {}
optional<shared_ptr<ArrayLiteral>> ArrayLiteral::parse(SyntaxContext& context, size_t& pos) {
    if (!AssertToken(context, pos, Token::Type::tkOpenBracket)) return {};
    const size_t startpos = pos++;
    vector<shared_ptr<Expression>> exprs;
    auto optexprs = CommaExpressions::parse(context, pos);
    if (optexprs.has_value()) exprs = optexprs.value()->expressions;
    if (!AssertToken(context, pos, Token::Type::tkClosedBracket)) {
        pos = startpos;
        return {};
    }
    ++pos;
    return make_shared<ArrayLiteral>(context.MakeSpanFromTokens(startpos, pos), exprs);
}
VISITOR(ArrayLiteral)
}  // namespace ast

optional<shared_ptr<ast::Body>> SyntaxAnalyzer::analyze(const vector<shared_ptr<Token>>& tokens,
                                                        const shared_ptr<const locators::CodeFile>& file,
                                                        complog::ICompilationLog& log) {
    SyntaxContext context(tokens, file, log);
    size_t pos = 0;
    auto res = ast::parseProgram(context, pos);
    if (!res.has_value())
        for (auto& err : context.report.messages) log.Log(err);
    return res;
}
