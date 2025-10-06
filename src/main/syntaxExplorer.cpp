#include "syntaxExplorer.h"
#include <sstream>
using namespace std;

static int StrToInt(const std::string& s) {
    istringstream ss(s);
    int res; ss >> res;
    return res;
}

#define EXPLORER_HELP(classname, _gotoactions, _GoTo, literalExplorer)\
class classname##literalExplorer : public ASTExplorer {\
    shared_ptr<ast::classname> node;\
public:\
    classname##literalExplorer(const shared_ptr<ast::classname>& node) : node(node) {}\
    vector<GotoAction> GetGotoActions() const override _gotoactions\
    shared_ptr<ast::ASTNode> GoTo(string command) const override _GoTo\
    string NodeName() const override { return #classname; }\
    virtual ~classname##literalExplorer() override = default;\
};

#define EXPLORER(classname, _gotoactions, _GoTo)\
    EXPLORER_HELP(classname, _gotoactions, _GoTo, Explorer)

EXPLORER(Body, { // GoToActions
    vector<GotoAction> res;
    int i = 0;
    for (const auto& pstatement : node->statements) {
        res.emplace_back(to_string(i), "statements[" + to_string(i) + "]");
        i++;
    }
    return res;
},
{  // GoTo
    return node->statements[StrToInt(command)];
})

class VarStatementExplorer : public ASTExplorer {
public:
    VarStatementExplorer(const std::shared_ptr<ast::VarStatement>& node) {

    }
};

class IfStatementExplorer : public ASTExplorer {
public:
    IfStatementExplorer(const std::shared_ptr<ast::IfStatement>& node) {

    }
};

class ShortIfStatementExplorer : public ASTExplorer {
public:
    ShortIfStatementExplorer(const std::shared_ptr<ast::ShortIfStatement>& node) {

    }
};

class WhileStatementExplorer : public ASTExplorer {
public:
    WhileStatementExplorer(const std::shared_ptr<ast::WhileStatement>& node) {

    }
};

class ForStatementExplorer : public ASTExplorer {
public:
    ForStatementExplorer(const std::shared_ptr<ast::ForStatement>& node) {

    }
};

class LoopStatementExplorer : public ASTExplorer {
public:
    LoopStatementExplorer(const std::shared_ptr<ast::LoopStatement>& node) {

    }
};

class ExitStatementExplorer : public ASTExplorer {
public:
    ExitStatementExplorer(const std::shared_ptr<ast::ExitStatement>& node) {

    }
};

class AssignStatementExplorer : public ASTExplorer {
public:
    AssignStatementExplorer(const std::shared_ptr<ast::AssignStatement>& node) {

    }
};

class PrintStatementExplorer : public ASTExplorer {
public:
    PrintStatementExplorer(const std::shared_ptr<ast::PrintStatement>& node) {

    }
};

class ReturnStatementExplorer : public ASTExplorer {
public:
    ReturnStatementExplorer(const std::shared_ptr<ast::ReturnStatement>& node) {

    }
};

class ExpressionStatementExplorer : public ASTExplorer {
public:
    ExpressionStatementExplorer(const std::shared_ptr<ast::ExpressionStatement>& node) {

    }
};

class EmptyStatementExplorer : public ASTExplorer {
public:
    EmptyStatementExplorer(const std::shared_ptr<ast::EmptyStatement>& node) {

    }
};

class CommaExpressionsExplorer : public ASTExplorer {
public:
    CommaExpressionsExplorer(const std::shared_ptr<ast::CommaExpressions>& node) {

    }
};

class CommaIdentsExplorer : public ASTExplorer {
public:
    CommaIdentsExplorer(const std::shared_ptr<ast::CommaIdents>& node) {

    }
};

class IdentMemberAccessorExplorer : public ASTExplorer {
public:
    IdentMemberAccessorExplorer(const std::shared_ptr<ast::IdentMemberAccessor>& node) {

    }
};

class IntLiteralMemberAccessorExplorer : public ASTExplorer {
public:
    IntLiteralMemberAccessorExplorer(const std::shared_ptr<ast::IntLiteralMemberAccessor>& node) {

    }
};

class ParenMemberAccessorExplorer : public ASTExplorer {
public:
    ParenMemberAccessorExplorer(const std::shared_ptr<ast::ParenMemberAccessor>& node) {

    }
};

class IndexAccessorExplorer : public ASTExplorer {
public:
    IndexAccessorExplorer(const std::shared_ptr<ast::IndexAccessor>& node) {

    }
};

class ReferenceExplorer : public ASTExplorer {
public:
    ReferenceExplorer(const std::shared_ptr<ast::Reference>& node) {

    }
};

class ExpressionExplorer : public ASTExplorer {
public:
    ExpressionExplorer(const std::shared_ptr<ast::Expression>& node) {

    }
};

class XorOperandExplorer : public ASTExplorer {
public:
    XorOperandExplorer(const std::shared_ptr<ast::XorOperand>& node) {

    }
};

class OrOperandExplorer : public ASTExplorer {
public:
    OrOperandExplorer(const std::shared_ptr<ast::OrOperand>& node) {

    }
};

class AndOperandExplorer : public ASTExplorer {
public:
    AndOperandExplorer(const std::shared_ptr<ast::AndOperand>& node) {

    }
};

class SumExplorer : public ASTExplorer {
public:
    SumExplorer(const std::shared_ptr<ast::Sum>& node) {

    }
};

class TermExplorer : public ASTExplorer {
public:
    TermExplorer(const std::shared_ptr<ast::Term>& node) {

    }
};

class UnaryExplorer : public ASTExplorer {
public:
    UnaryExplorer(const std::shared_ptr<ast::Unary>& node) {

    }
};

class PrefixOperatorExplorer : public ASTExplorer {
public:
    PrefixOperatorExplorer(const std::shared_ptr<ast::PrefixOperator>& node) {

    }
};

class TypecheckOperatorExplorer : public ASTExplorer {
public:
    TypecheckOperatorExplorer(const std::shared_ptr<ast::TypecheckOperator>& node) {

    }
};

class CallExplorer : public ASTExplorer {
public:
    CallExplorer(const std::shared_ptr<ast::Call>& node) {

    }
};

class AccessorOperatorExplorer : public ASTExplorer {
public:
    AccessorOperatorExplorer(const std::shared_ptr<ast::AccessorOperator>& node) {

    }
};

class PrimaryIdentExplorer : public ASTExplorer {
public:
    PrimaryIdentExplorer(const std::shared_ptr<ast::PrimaryIdent>& node) {

    }
};

class ParenthesesExpressionExplorer : public ASTExplorer {
public:
    ParenthesesExpressionExplorer(const std::shared_ptr<ast::ParenthesesExpression>& node) {

    }
};

class TupleLiteralElementExplorer : public ASTExplorer {
public:
    TupleLiteralElementExplorer(const std::shared_ptr<ast::TupleLiteralElement>& node) {

    }
};

class TupleLiteralExplorer : public ASTExplorer {
public:
    TupleLiteralExplorer(const std::shared_ptr<ast::TupleLiteral>& node) {

    }
};

class ShortFuncBodyExplorer : public ASTExplorer {
public:
    ShortFuncBodyExplorer(const std::shared_ptr<ast::ShortFuncBody>& node) {

    }
};

class LongFuncBodyExplorer : public ASTExplorer {
public:
    LongFuncBodyExplorer(const std::shared_ptr<ast::LongFuncBody>& node) {

    }
};

class FuncLiteralExplorer : public ASTExplorer {
public:
    FuncLiteralExplorer(const std::shared_ptr<ast::FuncLiteral>& node) {

    }
};

class TokenLiteralExplorer : public ASTExplorer {
public:
    TokenLiteralExplorer(const std::shared_ptr<ast::TokenLiteral>& node) {

    }
};

class ArrayLiteralExplorer : public ASTExplorer {
public:
    ArrayLiteralExplorer(const std::shared_ptr<ast::ArrayLiteral>& node) {

    }
};

shared_ptr<const ASTExplorer> ASTExplorerVisitor::MakeExplorer() { return this->explorer; }


#define VISITOR_HELP(classname, literalExplorer)\
void ASTExplorerVisitor::Visit##classname(ast::classname& node) {\
    this->explorer = make_shared<classname##literalExplorer>(make_shared<ast::classname>(node));\
}

#define VISITOR(classname) VISITOR_HELP(classname, Explorer)

VISITOR(Body)
VISITOR(VarStatement)
VISITOR(IfStatement)
VISITOR(ShortIfStatement)
VISITOR(WhileStatement)
VISITOR(ForStatement)
VISITOR(LoopStatement)
VISITOR(ExitStatement)
VISITOR(AssignStatement)
VISITOR(PrintStatement)
VISITOR(ReturnStatement)
VISITOR(ExpressionStatement)
VISITOR(EmptyStatement)
VISITOR(CommaExpressions)
VISITOR(CommaIdents)
VISITOR(IdentMemberAccessor)
VISITOR(IntLiteralMemberAccessor)
VISITOR(ParenMemberAccessor)
VISITOR(IndexAccessor)
VISITOR(Reference)
VISITOR(Expression)
VISITOR(XorOperand)
VISITOR(OrOperand)
VISITOR(AndOperand)
VISITOR(Sum)
VISITOR(Term)
VISITOR(Unary)
VISITOR(PrefixOperator)
VISITOR(TypecheckOperator)
VISITOR(Call)
VISITOR(AccessorOperator)
VISITOR(PrimaryIdent)
VISITOR(ParenthesesExpression)
VISITOR(TupleLiteralElement)
VISITOR(TupleLiteral)
VISITOR(ShortFuncBody)
VISITOR(LongFuncBody)
VISITOR(FuncLiteral)
VISITOR(TokenLiteral)
VISITOR(ArrayLiteral)

class ExplorerIO {
    shared_ptr<ast::ASTNode> rootNode;
public:
    ExplorerIO(const shared_ptr<ast::ASTNode>& root);
    static void PrintPrompt(const ASTExplorer& explorer);
    void Explore(ostream& output, istream& input);
};

