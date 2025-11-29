#include "dinterp/syntax.h"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <memory>
#include <stdexcept>
#include <variant>

#include "dinterp/asterrors.h"
#include "dinterp/complog/CompilationLog.h"
#include "dinterp/lexer.h"
#include "dinterp/locators/locator.h"
using namespace std;

namespace dinterp {
namespace ast {

#define VISITOR(classname) \
    void classname::AcceptVisitor(IASTVisitor& vis) { vis.Visit##classname(*this); }

ASTNode::ASTNode(const locators::SpanLocator& pos) : pos(pos) {}

#define USESCAN auto& tk = context.tokens

optional<shared_ptr<Body>> parseProgram(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStartUseEoln();
    vector<shared_ptr<Statement>> sts;

    while (true) {
        if (tk.Read(Token::Type::tkEof)) break;
        auto optStmt = Statement::parse(context);
        if (optStmt) sts.push_back(*optStmt);
        if (parseSep(context)) continue;
        if (tk.Read(Token::Type::tkEof)) break;
        return {};
    }
    auto res = make_shared<Body>(tk.ReadSinceStart());
    res->statements = sts;
    block.Success();
    return res;
}

bool parseSep(SyntaxContext& context) {
    USESCAN;
    return tk.Read(Token::Type::tkNewLine) || tk.Read(Token::Type::tkSemicolon);
}

optional<shared_ptr<Expression>> parseAssignExpression(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkAssign)) return {};
    auto res = Expression::parse(context);
    if (res) block.Success();
    return res;
}

Body::Body(const locators::SpanLocator& pos) : Statement(pos) {}
Body::Body(const locators::SpanLocator& pos, const vector<shared_ptr<Statement>>& statements)
    : Statement(pos), statements(statements) {}
optional<shared_ptr<Body>> Body::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStartUseEoln();
    vector<shared_ptr<Statement>> sts;
    while (true) {
        auto bl = tk.AutoStart();
        auto optStatement = Statement::parse(context);
        if (!parseSep(context)) break;
        if (optStatement) sts.push_back(*optStatement);
        bl.Success();
    }
    auto res = make_shared<Body>(tk.ReadSinceStart());
    res->statements = sts;
    block.Success();
    return res;
}
VISITOR(Body)

optional<shared_ptr<Body>> parseLoopBody(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkLoop)) return {};
    auto res = Body::parse(context);
    if (!res) return {};  // This is currently impossible. I am being explicit in case something changes.
    if (!tk.Read(Token::Type::tkEnd)) return {};
    block.Success();
    return res;
}

Statement::Statement(const locators::SpanLocator& pos) : ASTNode(pos) {}
optional<shared_ptr<Statement>> Statement::parse(SyntaxContext& context) {
#define TRY(classname)                        \
    {                                         \
        auto res = classname::parse(context); \
        if (res) return res;                  \
    }
    TRY(VarStatement)
    TRY(IfStatement)
    TRY(ShortIfStatement)
    TRY(WhileStatement)
    TRY(ForStatement)
    TRY(LoopStatement)
    TRY(ExitStatement)
    TRY(AssignStatement)
    TRY(PrintStatement)
    TRY(ReturnStatement)
    TRY(ExpressionStatement)
#undef TRY
    return {};
}

VarStatement::VarStatement(const locators::SpanLocator& pos) : Statement(pos) {}
optional<shared_ptr<VarStatement>> VarStatement::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkVar)) return {};
    tk.Read(Token::Type::tkNewLine);
    bool first = true;
    vector<pair<shared_ptr<IdentifierToken>, optional<shared_ptr<Expression>>>> defs;
    while (true) {
        auto bl = tk.AutoStart();
        if (!first) {
            if (!tk.Read(Token::Type::tkComma)) break;
            tk.Read(Token::Type::tkNewLine);
        }
        first = false;
        auto optIdent = tk.Read(Token::Type::tkIdent);
        if (!optIdent) break;
        auto tkIdent = dynamic_pointer_cast<IdentifierToken>(*optIdent);
        auto optAsg = parseAssignExpression(context);
        defs.emplace_back(tkIdent, optAsg);
        bl.Success();
    }
    if (defs.empty()) {
        context.compilationLog->Log(make_shared<EmptyVarStatement>(tk.StartPositionInFile()));
        return {};
    }
    auto res = make_shared<VarStatement>(tk.ReadSinceStart());
    res->definitions = defs;
    block.Success();
    return res;
}
VISITOR(VarStatement)

IfStatement::IfStatement(const locators::SpanLocator& pos, const shared_ptr<Expression>& condition,
                         const shared_ptr<Body>& doIfTrue, const optional<shared_ptr<Body>>& doIfFalse)
    : Statement(pos), condition(condition), doIfTrue(doIfTrue), doIfFalse(doIfFalse) {}
optional<shared_ptr<IfStatement>> IfStatement::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkIf)) return {};
    optional<shared_ptr<Expression>> optExpr;
    {
        auto ign = tk.AutoStartIgnoreEoln();
        optExpr = Expression::parse(context);
        if (!optExpr) return {};
        ign.Success();
    }
    if (!tk.Read(Token::Type::tkThen)) return {};
    auto optDoTrue = Body::parse(context);
    if (!optDoTrue) return {};
    optional<shared_ptr<Body>> optDoFalse;
    {
        auto bl = tk.AutoStart();
        if (tk.Read(Token::Type::tkElse)) optDoFalse = Body::parse(context);
        if (optDoFalse) bl.Success();
    }
    if (!tk.Read(Token::Type::tkEnd)) return {};
    block.Success();
    return make_shared<IfStatement>(tk.ReadSinceStart(), *optExpr, *optDoTrue, optDoFalse);
}
VISITOR(IfStatement)

ShortIfStatement::ShortIfStatement(const locators::SpanLocator& pos, const shared_ptr<Expression>& condition,
                                   const shared_ptr<Statement>& doIfTrue)
    : Statement(pos), condition(condition), doIfTrue(doIfTrue) {}
optional<shared_ptr<ShortIfStatement>> ShortIfStatement::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    optional<shared_ptr<Expression>> optExpr;
    if (!tk.Read(Token::Type::tkIf)) return {};
    {
        auto ign = tk.AutoStartIgnoreEoln();
        optExpr = Expression::parse(context);
        if (!optExpr) return {};
        ign.Success();
    }
    if (!tk.Read(Token::Type::tkArrow)) return {};
    tk.Read(Token::Type::tkNewLine);
    auto optStatement = Statement::parse(context);
    if (!optStatement) return {};
    block.Success();
    return make_shared<ShortIfStatement>(tk.ReadSinceStart(), *optExpr, *optStatement);
}
VISITOR(ShortIfStatement)

WhileStatement::WhileStatement(const locators::SpanLocator& pos, const shared_ptr<Expression>& condition,
                               const shared_ptr<Body>& action)
    : Statement(pos), condition(condition), action(action) {}
optional<shared_ptr<WhileStatement>> WhileStatement::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkWhile)) return {};
    optional<shared_ptr<Expression>> optExpr;
    {
        auto ign = tk.AutoStartIgnoreEoln();
        optExpr = Expression::parse(context);
        if (!optExpr) return {};
        ign.Success();
    }
    auto optBody = parseLoopBody(context);
    if (!optBody) return {};
    block.Success();
    return make_shared<WhileStatement>(tk.ReadSinceStart(), *optExpr, *optBody);
}
VISITOR(WhileStatement)

ForStatement::ForStatement(const locators::SpanLocator& pos) : Statement(pos) {}
optional<shared_ptr<ForStatement>> ForStatement::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkFor)) return {};
    optional<shared_ptr<IdentifierToken>> optVarName;
    {
        auto bl = tk.AutoStart();
        auto optIdent = tk.Read(Token::Type::tkIdent);
        if (optIdent && tk.Read(Token::Type::tkIn)) {
            optVarName = dynamic_pointer_cast<IdentifierToken>(*optIdent);
            bl.Success();
        }
    }
    optional<shared_ptr<Expression>> startOrList;
    {
        auto ign = tk.AutoStartIgnoreEoln();
        startOrList = Expression::parse(context);
        if (!startOrList) return {};
        ign.Success();
    }
    optional<shared_ptr<Expression>> rangeEnd;
    {
        auto bl = tk.AutoStart();
        if (tk.Read(Token::Type::tkRange)) {
            auto ign = tk.AutoStartIgnoreEoln();
            rangeEnd = Expression::parse(context);
            if (rangeEnd) ign.Success();
        }
        if (rangeEnd) bl.Success();
    }
    auto action = parseLoopBody(context);
    if (!action) return {};
    auto res = make_shared<ForStatement>(tk.ReadSinceStart());
    res->optVariableName = optVarName;
    res->startOrList = *startOrList;
    res->end = rangeEnd;
    res->action = *action;
    block.Success();
    return res;
}
VISITOR(ForStatement)

LoopStatement::LoopStatement(const locators::SpanLocator& pos, const shared_ptr<Body>& body)
    : Statement(pos), body(body) {}
optional<shared_ptr<LoopStatement>> LoopStatement::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    auto body = parseLoopBody(context);
    if (!body) return {};
    block.Success();
    return make_shared<LoopStatement>(tk.ReadSinceStart(), *body);
}
VISITOR(LoopStatement)

ExitStatement::ExitStatement(const locators::SpanLocator& pos) : Statement(pos) {}
optional<shared_ptr<ExitStatement>> ExitStatement::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkExit)) return {};
    block.Success();
    return make_shared<ExitStatement>(tk.ReadSinceStart());
}
VISITOR(ExitStatement)

AssignStatement::AssignStatement(const locators::SpanLocator& pos, const shared_ptr<Reference>& dest,
                                 const shared_ptr<Expression>& src)
    : Statement(pos), dest(dest), src(src) {}
optional<shared_ptr<AssignStatement>> AssignStatement::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    auto optRef = Reference::parse(context);
    if (!optRef) return {};
    if (!tk.Read(Token::Type::tkAssign)) return {};
    auto optExpr = Expression::parse(context);
    if (!optExpr) return {};
    block.Success();
    return make_shared<AssignStatement>(tk.ReadSinceStart(), *optRef, *optExpr);
}
VISITOR(AssignStatement)

PrintStatement::PrintStatement(const locators::SpanLocator& pos, const vector<shared_ptr<Expression>>& expressions)
    : Statement(pos), expressions(expressions) {}
optional<shared_ptr<PrintStatement>> PrintStatement::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkPrint)) return {};
    block.Success();
    auto exprs = CommaExpressions::parse(context);
    vector<shared_ptr<Expression>> lst;
    if (exprs) lst = exprs.value()->expressions;
    return make_shared<PrintStatement>(tk.ReadSinceStart(), lst);
}
VISITOR(PrintStatement)

ReturnStatement::ReturnStatement(const locators::SpanLocator& pos, const optional<shared_ptr<Expression>>& returnValue)
    : Statement(pos), returnValue(returnValue) {}
optional<shared_ptr<ReturnStatement>> ReturnStatement::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkReturn)) return {};
    block.Success();
    auto optExpr = Expression::parse(context);
    return make_shared<ReturnStatement>(tk.ReadSinceStart(), optExpr);
}
VISITOR(ReturnStatement)

ExpressionStatement::ExpressionStatement(const locators::SpanLocator& pos, const shared_ptr<Expression>& expr)
    : Statement(pos), expr(expr) {}
optional<shared_ptr<ExpressionStatement>> ExpressionStatement::parse(SyntaxContext& context) {
    auto res = Expression::parse(context);
    if (!res) return {};
    return make_shared<ExpressionStatement>(res.value()->pos, res.value());
}
VISITOR(ExpressionStatement)

CommaExpressions::CommaExpressions(const locators::SpanLocator& pos, const vector<shared_ptr<Expression>>& expressions)
    : ASTNode(pos), expressions(expressions) {}
optional<shared_ptr<CommaExpressions>> CommaExpressions::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    bool first = true;
    vector<shared_ptr<Expression>> exprs;
    while (true) {
        auto bl = tk.AutoStart();
        if (!first) {
            if (!tk.Read(Token::Type::tkComma)) break;
        }
        first = false;
        auto optExpr = Expression::parse(context);
        if (!optExpr) break;
        bl.Success();
        exprs.push_back(*optExpr);
    }
    if (exprs.empty()) return {};
    block.Success();
    return make_shared<CommaExpressions>(tk.ReadSinceStart(), exprs);
}
VISITOR(CommaExpressions)

CommaIdents::CommaIdents(const locators::SpanLocator& pos, const vector<shared_ptr<IdentifierToken>>& idents)
    : ASTNode(pos), idents(idents) {}
optional<shared_ptr<CommaIdents>> CommaIdents::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    bool first = true;
    vector<shared_ptr<IdentifierToken>> tokens;
    while (true) {
        auto bl = tk.AutoStart();
        if (!first) {
            if (!tk.Read(Token::Type::tkComma)) break;
        }
        first = false;
        auto optIdent = tk.Read(Token::Type::tkIdent);
        if (!optIdent) break;
        auto ident = dynamic_pointer_cast<IdentifierToken>(*optIdent);
        bl.Success();
        tokens.push_back(ident);
    }
    if (tokens.empty()) return {};
    block.Success();
    return make_shared<CommaIdents>(tk.ReadSinceStart(), tokens);
}
VISITOR(CommaIdents)

Accessor::Accessor(const locators::SpanLocator& pos) : ASTNode(pos) {}
optional<shared_ptr<Accessor>> Accessor::parse(SyntaxContext& context) {
    {
        auto res = IdentMemberAccessor::parse(context);
        if (res) return res;
    }
    {
        auto res = IntLiteralMemberAccessor::parse(context);
        if (res) return res;
    }
    {
        auto res = ParenMemberAccessor::parse(context);
        if (res) return res;
    }
    {
        auto res = IndexAccessor::parse(context);
        if (res) return res;
    }
    return {};
}

IdentMemberAccessor::IdentMemberAccessor(const locators::SpanLocator& pos, const shared_ptr<IdentifierToken>& name)
    : Accessor(pos), name(name) {}
optional<shared_ptr<IdentMemberAccessor>> IdentMemberAccessor::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkDot)) return {};
    auto optIdent = tk.Read(Token::Type::tkIdent);
    if (!optIdent) return {};
    auto ident = dynamic_pointer_cast<IdentifierToken>(*optIdent);
    block.Success();
    return make_shared<IdentMemberAccessor>(tk.ReadSinceStart(), ident);
}
VISITOR(IdentMemberAccessor)

IntLiteralMemberAccessor::IntLiteralMemberAccessor(const locators::SpanLocator& pos,
                                                   const shared_ptr<IntegerToken>& index)
    : Accessor(pos), index(index) {}
optional<shared_ptr<IntLiteralMemberAccessor>> IntLiteralMemberAccessor::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkDot)) return {};
    auto optint = tk.Read(Token::Type::tkIntLiteral);
    if (!optint) return {};
    auto intlit = dynamic_pointer_cast<IntegerToken>(*optint);
    block.Success();
    return make_shared<IntLiteralMemberAccessor>(tk.ReadSinceStart(), intlit);
}
VISITOR(IntLiteralMemberAccessor)

ParenMemberAccessor::ParenMemberAccessor(const locators::SpanLocator& pos, const shared_ptr<Expression>& expr)
    : Accessor(pos), expr(expr) {}
optional<shared_ptr<ParenMemberAccessor>> ParenMemberAccessor::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkDot)) return {};
    auto expr = ParenthesesExpression::parse(context);
    if (!expr) return {};
    block.Success();
    return make_shared<ParenMemberAccessor>(tk.ReadSinceStart(), expr.value()->expr);
}
VISITOR(ParenMemberAccessor)

IndexAccessor::IndexAccessor(const locators::SpanLocator& pos, const shared_ptr<Expression>& expressionInBrackets)
    : Accessor(pos), expressionInBrackets(expressionInBrackets) {}
optional<shared_ptr<IndexAccessor>> IndexAccessor::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkOpenBracket)) return {};
    optional<shared_ptr<Expression>> expr;
    {
        auto ign = tk.AutoStartIgnoreEoln();
        expr = Expression::parse(context);
        if (!expr) return {};
        ign.Success();
    }
    if (!tk.Read(Token::Type::tkClosedBracket)) return {};
    block.Success();
    return make_shared<IndexAccessor>(tk.ReadSinceStart(), *expr);
}
VISITOR(IndexAccessor)

Reference::Reference(const locators::SpanLocator& pos, const shared_ptr<IdentifierToken>& baseIdent,
                     const vector<shared_ptr<Accessor>>& accessorChain)
    : ASTNode(pos), baseIdent(baseIdent), accessorChain(accessorChain) {}
optional<shared_ptr<Reference>> Reference::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    auto optIdent = tk.Read(Token::Type::tkIdent);
    if (!optIdent) return {};
    auto ident = dynamic_pointer_cast<IdentifierToken>(*optIdent);
    vector<shared_ptr<Accessor>> chain;
    while (true) {
        auto acc = Accessor::parse(context);
        if (!acc) break;
        chain.push_back(*acc);
    }
    block.Success();
    return make_shared<Reference>(tk.ReadSinceStart(), ident, chain);
}
VISITOR(Reference)

enum class BinaryPrecedence { Mul, Sum, Comparison, Not, And, Or, Xor };

struct ParsedBinaryOperator {
    BinaryPrecedence Precedence;
    std::optional<std::variant<Term::TermOperator, Sum::SumOperator, BinaryRelationOperator>> OpKind;
};

optional<ParsedBinaryOperator> parseBinaryOperator(SyntaxContext& context) {
    USESCAN;
    if (tk.Read(Token::Type::tkTimes)) return {{BinaryPrecedence::Mul, Term::TermOperator::Times}};
    if (tk.Read(Token::Type::tkDivide)) return {{BinaryPrecedence::Mul, Term::TermOperator::Divide}};
    if (tk.Read(Token::Type::tkPlus)) return {{BinaryPrecedence::Sum, Sum::SumOperator::Plus}};
    if (tk.Read(Token::Type::tkMinus)) return {{BinaryPrecedence::Sum, Sum::SumOperator::Minus}};
    if (tk.Read(Token::Type::tkLess)) return {{BinaryPrecedence::Comparison, BinaryRelationOperator::Less}};
    if (tk.Read(Token::Type::tkLessEq)) return {{BinaryPrecedence::Comparison, BinaryRelationOperator::LessEq}};
    if (tk.Read(Token::Type::tkGreater)) return {{BinaryPrecedence::Comparison, BinaryRelationOperator::Greater}};
    if (tk.Read(Token::Type::tkGreaterEq)) return {{BinaryPrecedence::Comparison, BinaryRelationOperator::GreaterEq}};
    if (tk.Read(Token::Type::tkEqual)) return {{BinaryPrecedence::Comparison, BinaryRelationOperator::Equal}};
    if (tk.Read(Token::Type::tkNotEqual)) return {{BinaryPrecedence::Comparison, BinaryRelationOperator::NotEqual}};
    if (tk.Read(Token::Type::tkAnd)) return {{BinaryPrecedence::And, {}}};
    if (tk.Read(Token::Type::tkOr)) return {{BinaryPrecedence::Or, {}}};
    if (tk.Read(Token::Type::tkXor)) return {{BinaryPrecedence::Xor, {}}};
    return {};
}

static void CollapseOperandStack(vector<shared_ptr<Expression>>& operands, vector<ParsedBinaryOperator>& operators,
                                 int minPrecedence) {
    while (operators.size() && static_cast<int>(operators.back().Precedence) < minPrecedence) {
        auto precedence = operators.back().Precedence;
        size_t count = 2;
        size_t n_operands = operands.size(), n_operators = operators.size();
        while (count < n_operands && operators[n_operators - count].Precedence == precedence) ++count;
        vector<shared_ptr<Expression>> nds(operands.end() - count, operands.end());
        operands.erase(operands.end() - count, operands.end());
        shared_ptr<Expression> composite;
        switch (precedence) {
            case BinaryPrecedence::Mul: {
                vector<Term::TermOperator> op(count - 1);
                std::ranges::transform(operators.end() - (count - 1), operators.end(), op.begin(),
                                       [](ParsedBinaryOperator p) { return get<Term::TermOperator>(*p.OpKind); });
                composite = make_shared<Term>(nds, op);
                break;
            }
            case BinaryPrecedence::Sum: {
                vector<Sum::SumOperator> op(count - 1);
                std::ranges::transform(operators.end() - (count - 1), operators.end(), op.begin(),
                                       [](ParsedBinaryOperator p) { return get<Sum::SumOperator>(*p.OpKind); });
                composite = make_shared<Sum>(nds, op);
                break;
            }
            case BinaryPrecedence::Comparison: {
                vector<BinaryRelationOperator> op(count - 1);
                std::ranges::transform(operators.end() - (count - 1), operators.end(), op.begin(),
                                       [](ParsedBinaryOperator p) { return get<BinaryRelationOperator>(*p.OpKind); });
                composite = make_shared<BinaryRelation>(nds, op);
                break;
            }
            case BinaryPrecedence::And: {
                composite = make_shared<AndOperator>(nds);
                break;
            }
            case BinaryPrecedence::Or: {
                composite = make_shared<OrOperator>(nds);
                break;
            }
            case BinaryPrecedence::Xor: {
                composite = make_shared<XorOperator>(nds);
                break;
            }
            default:
                throw invalid_argument("NOT operator is not a binary operator");
        }
        operands.push_back(composite);
        operators.erase(operators.end() - (count - 1), operators.end());
    }
}

Expression::Expression(const locators::SpanLocator& pos) : ASTNode(pos) {}

optional<shared_ptr<Expression>> Expression::parse(SyntaxContext& context, int max_precedence) {
    USESCAN;
    auto block = tk.AutoStart();
    bool first = true;
    vector<shared_ptr<Expression>> unaries;
    vector<ParsedBinaryOperator> operators;
    while (true) {
        auto bl = tk.AutoStart();
        ParsedBinaryOperator op;
        if (!first) {
            auto opt_op = parseBinaryOperator(context);
            if (!opt_op) break;
            if (static_cast<int>(opt_op->Precedence) > max_precedence) break;
            op = *opt_op;
        }
        optional<shared_ptr<Expression>> optunary = UnaryNot::parse(context);
        if (!optunary) {
            optunary = Unary::parse(context);
            if (!optunary) break;
        }
        bl.Success();

        if (first) {
            first = false;
            unaries.push_back(*optunary);
            continue;
        }
        CollapseOperandStack(unaries, operators, static_cast<int>(op.Precedence));
        unaries.push_back(*optunary);
        operators.push_back(op);
    }
    if (first) return {};
    CollapseOperandStack(unaries, operators, numeric_limits<int>::max());
    block.Success();
    return unaries.front();
}

locators::SpanLocator SpanLocatorFromExpressions(const Expression& first, const Expression& last) {
    auto file = first.pos.File();
    size_t start = first.pos.Start().Position();
    size_t end = last.pos.End().Position();
    return {file, start, end - start};
}

locators::SpanLocator SpanLocatorFromExpressions(const vector<shared_ptr<Expression>>& exprs) {
    return SpanLocatorFromExpressions(*exprs.front(), *exprs.back());
}

XorOperator::XorOperator(const vector<shared_ptr<Expression>>& operands)
    : Expression(SpanLocatorFromExpressions(operands)), operands(operands) {}
VISITOR(XorOperator)

OrOperator::OrOperator(const vector<shared_ptr<Expression>>& operands)
    : Expression(SpanLocatorFromExpressions(operands)), operands(operands) {}
VISITOR(OrOperator)

AndOperator::AndOperator(const vector<shared_ptr<Expression>>& operands)
    : Expression(SpanLocatorFromExpressions(operands)), operands(operands) {}
VISITOR(AndOperator)

BinaryRelation::BinaryRelation(const vector<shared_ptr<Expression>>& operands,
                               const vector<BinaryRelationOperator>& operators)
    : Expression(SpanLocatorFromExpressions(operands)), operands(operands), operators(operators) {
    size_t nds = operands.size(), ors = operators.size();
    if (nds != ors + 1) throw WrongNumberOfOperatorsSupplied("BinaryRelation", nds, ors);
}
VISITOR(BinaryRelation)

optional<BinaryRelationOperator> parseBinaryRelationOperator(SyntaxContext& context) {
    USESCAN;
#define TRY(op) \
    if (tk.Read(Token::Type::tk##op)) return BinaryRelationOperator::op;
    TRY(Less)
    TRY(LessEq)
    TRY(Greater)
    TRY(GreaterEq)
    TRY(Equal)
    TRY(NotEqual)
    return {};
#undef TRY
}

Sum::Sum(const vector<shared_ptr<Expression>>& terms, const vector<SumOperator>& operators)
    : Expression(SpanLocatorFromExpressions(terms)), terms(terms), operators(operators) {
    size_t nds = terms.size(), ors = operators.size();
    if (nds != ors + 1) throw WrongNumberOfOperatorsSupplied("Sum", nds, ors);
}
VISITOR(Sum)

Term::Term(const vector<shared_ptr<Expression>>& unaries, const vector<TermOperator>& operators)
    : Expression(SpanLocatorFromExpressions(unaries)), unaries(unaries), operators(operators) {
    size_t nds = unaries.size(), ors = operators.size();
    if (nds != ors + 1) throw WrongNumberOfOperatorsSupplied("Term", nds, ors);
}
VISITOR(Term)

Unary::Unary(const locators::SpanLocator& pos, const vector<shared_ptr<PrefixOperator>>& prefixOps,
             const vector<shared_ptr<PostfixOperator>>& postfixOps, const shared_ptr<Expression>& expr)
    : Expression(pos), prefixOps(prefixOps), postfixOps(postfixOps), expr(expr) {}
optional<shared_ptr<Unary>> Unary::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    vector<shared_ptr<PrefixOperator>> prefs;
    while (true) {
        auto op = PrefixOperator::parse(context);
        if (!op) break;
        prefs.push_back(*op);
    }
    auto prim = Primary::parse(context);
    if (!prim) return {};
    vector<shared_ptr<PostfixOperator>> posts;
    while (true) {
        auto op = PostfixOperator::parse(context);
        if (!op) break;
        posts.push_back(*op);
    }
    block.Success();
    return make_shared<Unary>(tk.ReadSinceStart(), prefs, posts, *prim);
}
VISITOR(Unary)

UnaryNot::UnaryNot(const locators::SpanLocator& pos, const shared_ptr<Expression>& nested)
    : Expression(pos), nested(nested) {}
optional<shared_ptr<UnaryNot>> UnaryNot::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkNot)) return {};
    auto nested = Expression::parse(context, static_cast<int>(BinaryPrecedence::Not));
    if (!nested) return {};
    block.Success();
    return make_shared<UnaryNot>(tk.ReadSinceStart(), *nested);
}
VISITOR(UnaryNot)

PrefixOperator::PrefixOperator(const locators::SpanLocator& pos, PrefixOperatorKind kind) : ASTNode(pos), kind(kind) {}
int PrefixOperator::precedence() {  // the less, the more priority
    return 2;
}
VISITOR(PrefixOperator)
optional<shared_ptr<PrefixOperator>> PrefixOperator::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    PrefixOperatorKind kind;
    if (tk.Read(Token::Type::tkMinus))
        kind = PrefixOperatorKind::Minus;
    else if (tk.Read(Token::Type::tkPlus))
        kind = PrefixOperatorKind::Plus;
    else
        return {};
    block.Success();
    return make_shared<PrefixOperator>(tk.ReadSinceStart(), kind);
}

PostfixOperator::PostfixOperator(const locators::SpanLocator& pos) : ASTNode(pos) {}
optional<shared_ptr<PostfixOperator>> PostfixOperator::parse(SyntaxContext& context) {
#define TRY(classname)                        \
    {                                         \
        auto res = classname::parse(context); \
        if (res) return res;                  \
    }
    TRY(TypecheckOperator)
    TRY(Call)
    TRY(AccessorOperator)
    return {};
#undef TRY
}

TypecheckOperator::TypecheckOperator(const locators::SpanLocator& pos, TypeId typeId)
    : PostfixOperator(pos), typeId(typeId) {}
int TypecheckOperator::precedence() { return 3; }
optional<shared_ptr<TypecheckOperator>> TypecheckOperator::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkIs)) return {};
    auto op = parseTypeId(context);
    if (!op) return {};
    block.Success();
    return make_shared<TypecheckOperator>(tk.ReadSinceStart(), *op);
}
VISITOR(TypecheckOperator)

optional<TypeId> parseTypeId(SyntaxContext& context) {
    USESCAN;
#define TRY(name)                         \
    if (tk.Read(Token::Type::tk##name)) { \
        return TypeId::name;              \
    }
    TRY(Int)
    TRY(Real)
    TRY(String)
    TRY(Bool)
    TRY(None)
    TRY(Func)
#undef TRY
    {
        auto bl = tk.AutoStart();
        if (tk.Read(Token::Type::tkOpenBracket) && tk.Read(Token::Type::tkClosedBracket)) {
            bl.Success();
            return TypeId::List;
        }
    }
    {
        auto bl = tk.AutoStart();
        if (tk.Read(Token::Type::tkOpenCurlyBrace) && tk.Read(Token::Type::tkClosedCurlyBrace)) {
            bl.Success();
            return TypeId::Tuple;
        }
    }
    return {};
}

Call::Call(const locators::SpanLocator& pos, const vector<shared_ptr<Expression>>& args)
    : PostfixOperator(pos), args(args) {}
int Call::precedence() { return 1; }
optional<shared_ptr<Call>> Call::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    optional<shared_ptr<CommaExpressions>> comexpr;
    if (!tk.Read(Token::Type::tkOpenParenthesis)) return {};
    {
        auto ign = tk.AutoStartIgnoreEoln();
        comexpr = CommaExpressions::parse(context);
        ign.Success();
    }
    vector<shared_ptr<Expression>> args;
    if (comexpr) args = comexpr.value()->expressions;
    if (!tk.Read(Token::Type::tkClosedParenthesis)) return {};
    block.Success();
    return make_shared<Call>(tk.ReadSinceStart(), args);
}
VISITOR(Call)

AccessorOperator::AccessorOperator(const locators::SpanLocator& pos, const shared_ptr<Accessor>& accessor)
    : PostfixOperator(pos), accessor(accessor) {}
int AccessorOperator::precedence() { return 1; }
optional<shared_ptr<AccessorOperator>> AccessorOperator::parse(SyntaxContext& context) {
    auto res = Accessor::parse(context);
    if (!res) return {};
    return make_shared<AccessorOperator>(res.value()->pos, res.value());
}
VISITOR(AccessorOperator)

Primary::Primary(const locators::SpanLocator& pos) : Expression(pos) {}
optional<shared_ptr<Primary>> Primary::parse(SyntaxContext& context) {
#define TRY(classname)                        \
    {                                         \
        auto res = classname::parse(context); \
        if (res) return res;                  \
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

PrimaryIdent::PrimaryIdent(const locators::SpanLocator& pos, const shared_ptr<IdentifierToken>& name)
    : Primary(pos), name(name) {}
optional<shared_ptr<PrimaryIdent>> PrimaryIdent::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    auto optIdent = tk.Read(Token::Type::tkIdent);
    if (!optIdent) return {};
    block.Success();
    return make_shared<PrimaryIdent>(tk.ReadSinceStart(), dynamic_pointer_cast<IdentifierToken>(*optIdent));
}
VISITOR(PrimaryIdent)

ParenthesesExpression::ParenthesesExpression(const locators::SpanLocator& pos, const shared_ptr<Expression>& expr)
    : Primary(pos), expr(expr) {}
optional<shared_ptr<ParenthesesExpression>> ParenthesesExpression::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkOpenParenthesis)) return {};
    optional<shared_ptr<Expression>> expr;
    {
        auto ign = tk.AutoStartIgnoreEoln();
        expr = Expression::parse(context);
        if (!expr) return {};
        ign.Success();
    }
    if (!tk.Read(Token::Type::tkClosedParenthesis)) return {};
    block.Success();
    return make_shared<ParenthesesExpression>(tk.ReadSinceStart(), *expr);
}
VISITOR(ParenthesesExpression)

TupleLiteralElement::TupleLiteralElement(const locators::SpanLocator& pos,
                                         const optional<shared_ptr<IdentifierToken>>& ident,
                                         const shared_ptr<Expression>& expression)
    : ASTNode(pos), ident(ident), expression(expression) {}
optional<shared_ptr<TupleLiteralElement>> TupleLiteralElement::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    optional<shared_ptr<IdentifierToken>> ident;
    {
        auto bl = tk.AutoStart();
        auto optident = tk.Read(Token::Type::tkIdent);
        if (optident) {
            if (tk.Read(Token::Type::tkAssign)) {
                bl.Success();
                ident = dynamic_pointer_cast<IdentifierToken>(*optident);
            }
        }
    }
    auto optExpr = Expression::parse(context);
    if (!optExpr) return {};
    block.Success();
    return make_shared<TupleLiteralElement>(tk.ReadSinceStart(), ident, *optExpr);
}
VISITOR(TupleLiteralElement)

TupleLiteral::TupleLiteral(const locators::SpanLocator& pos, const vector<shared_ptr<TupleLiteralElement>>& elements)
    : Primary(pos), elements(elements) {}
optional<shared_ptr<TupleLiteral>> TupleLiteral::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkOpenCurlyBrace)) return {};
    vector<shared_ptr<TupleLiteralElement>> elems;
    bool first = true;
    {
        auto ign = tk.AutoStartIgnoreEoln();
        ign.Success();
        while (true) {
            auto bl = tk.AutoStart();
            if (!first) {
                if (!tk.Read(Token::Type::tkComma)) break;
            }
            first = false;
            auto elem = TupleLiteralElement::parse(context);
            if (!elem) break;
            elems.push_back(*elem);
            bl.Success();
        }
    }
    if (!tk.Read(Token::Type::tkClosedCurlyBrace)) return {};
    block.Success();
    return make_shared<TupleLiteral>(tk.ReadSinceStart(), elems);
}
VISITOR(TupleLiteral)

FuncBody::FuncBody(const locators::SpanLocator& pos) : ASTNode(pos) {}
optional<shared_ptr<FuncBody>> FuncBody::parse(SyntaxContext& context) {
#define TRY(classname)                        \
    {                                         \
        auto res = classname::parse(context); \
        if (res) return res;                  \
    }
    TRY(ShortFuncBody)
    TRY(LongFuncBody)
#undef TRY
    return {};
}

ShortFuncBody::ShortFuncBody(const locators::SpanLocator& pos, const shared_ptr<Expression>& expressionToReturn)
    : FuncBody(pos), expressionToReturn(expressionToReturn) {}
optional<shared_ptr<ShortFuncBody>> ShortFuncBody::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkArrow)) return {};
    auto optExpr = Expression::parse(context);
    if (!optExpr) return {};
    block.Success();
    return make_shared<ShortFuncBody>(tk.ReadSinceStart(), *optExpr);
}
VISITOR(ShortFuncBody)

LongFuncBody::LongFuncBody(const locators::SpanLocator& pos, const shared_ptr<Body>& funcBody)
    : FuncBody(pos), funcBody(funcBody) {}
optional<shared_ptr<LongFuncBody>> LongFuncBody::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkIs)) return {};
    auto optBody = Body::parse(context);
    if (!optBody) return {};
    if (!tk.Read(Token::Type::tkEnd)) return {};
    block.Success();
    return make_shared<LongFuncBody>(tk.ReadSinceStart(), *optBody);
}
VISITOR(LongFuncBody)

FuncLiteral::FuncLiteral(const locators::SpanLocator& pos, const vector<shared_ptr<IdentifierToken>>& parameters,
                         const shared_ptr<FuncBody>& funcBody)
    : Primary(pos), parameters(parameters), funcBody(funcBody) {}
optional<shared_ptr<FuncLiteral>> FuncLiteral::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkFunc)) return {};
    if (!tk.Read(Token::Type::tkOpenParenthesis)) return {};
    vector<shared_ptr<IdentifierToken>> params;
    {
        auto ign = tk.AutoStartIgnoreEoln();
        ign.Success();
        auto optIdents = CommaIdents::parse(context);
        if (optIdents) params = optIdents.value()->idents;
    }
    if (!tk.Read(Token::Type::tkClosedParenthesis)) return {};
    auto body = FuncBody::parse(context);
    if (!body) return {};
    block.Success();
    return make_shared<FuncLiteral>(tk.ReadSinceStart(), params, *body);
}
VISITOR(FuncLiteral)

TokenLiteral::TokenLiteral(const locators::SpanLocator& pos, TokenLiteralKind kind, const shared_ptr<Token>& token)
    : Primary(pos), kind(kind), token(token) {}
optional<shared_ptr<TokenLiteral>> TokenLiteral::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
#define TRY(tktype, reskind)                        \
    if ((token = tk.Read(Token::Type::tk##tktype))) \
        kind = TokenLiteralKind::reskind;           \
    else
    TokenLiteralKind kind;
    optional<shared_ptr<Token>> token;
    TRY(StringLiteral, String)
    TRY(IntLiteral, Int)
    TRY(RealLiteral, Real)
    TRY(True, True)
    TRY(False, False)
    TRY(None, None)
    return {};
#undef TRY
    block.Success();
    return make_shared<TokenLiteral>(tk.ReadSinceStart(), kind, *token);
}
VISITOR(TokenLiteral)

ArrayLiteral::ArrayLiteral(const locators::SpanLocator& pos, const vector<shared_ptr<Expression>>& items)
    : Primary(pos), items(items) {}
optional<shared_ptr<ArrayLiteral>> ArrayLiteral::parse(SyntaxContext& context) {
    USESCAN;
    auto block = tk.AutoStart();
    if (!tk.Read(Token::Type::tkOpenBracket)) return {};
    vector<shared_ptr<Expression>> exprs;
    {
        auto ign = tk.AutoStartIgnoreEoln();
        ign.Success();
        auto optexprs = CommaExpressions::parse(context);
        if (optexprs) exprs = optexprs.value()->expressions;
    }
    if (!tk.Read(Token::Type::tkClosedBracket)) return {};
    block.Success();
    return make_shared<ArrayLiteral>(tk.ReadSinceStart(), exprs);
}
VISITOR(ArrayLiteral)
}  // namespace ast

optional<shared_ptr<ast::Body>> SyntaxAnalyzer::analyze(const vector<shared_ptr<Token>>& tokens,
                                                        const shared_ptr<const locators::CodeFile>& file,
                                                        complog::ICompilationLog& log) {
    SyntaxContext context(tokens, file, log);
    auto res = ast::parseProgram(context);
    if (!res)
        for (auto& err : context.tokens.Report().MakeReport()) log.Log(err);
    return res;
}
}  // namespace dinterp
