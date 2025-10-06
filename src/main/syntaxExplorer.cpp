#include "syntaxExplorer.h"
#include <sstream>
using namespace std;

static int StrToInt(const std::string& s) {
    istringstream ss(s);
    int res; ss >> res;
    return res;
}

#define EXPLORER_HELP(classname, _gotoactions, _action, literalExplorer)\
class classname##literalExplorer : public ASTExplorer {\
    shared_ptr<ast::classname> node;\
public:\
    classname##literalExplorer(const shared_ptr<ast::classname>& node) : node(node) {}\
    vector<GotoAction> GetGotoActions() const override _gotoactions\
    optional<shared_ptr<ast::ASTNode>> Action(string command, [[maybe_unused]] ostream& output) const override _action\
    string NodeName() const override { return #classname; }\
    virtual ~classname##literalExplorer() override = default;\
};

#define EXPLORER(classname, _gotoactions, _action)\
    EXPLORER_HELP(classname, _gotoactions, _action, Explorer)

EXPLORER(Body, { // GoToActions
    vector<GotoAction> res;
    int i = 0;
    for (const auto& pstatement : node->statements) {
        res.emplace_back(to_string(i), "statements[" + to_string(i) + "]");
        i++;
    }
    return res;
},
{  // Action
    return node->statements[StrToInt(command)];
})

EXPLORER(VarStatement, {  // GoToActions
    vector<GotoAction> res;
    int i = 0;
    for (const auto& p : node->definitions) {
        res.emplace_back("n" + to_string(i), "Name  of definitions[" + to_string(i) + "]");
        if (p.second.has_value())
            res.emplace_back("v" + to_string(i), "Value of definitions[" + to_string(i) + "]");
        i++;
    }
    return res;
},
{  // Action
    bool name = command[0] == 'n';
    command.erase(0, 1);
    auto& p = node->definitions[StrToInt(command)];
    if (name) {
        output << p.first;
        return {};
    }
    else
        return *p.second;
})

EXPLORER(IfStatement, {  // GoToActions

},
{  // Action

})

EXPLORER(ShortIfStatement, {  // GoToActions

},
{  // Action

})

EXPLORER(WhileStatement, {  // GoToActions

},
{  // Action

})

EXPLORER(ForStatement, {  // GoToActions

},
{  // Action

})

EXPLORER(LoopStatement, {  // GoToActions

},
{  // Action

})

EXPLORER(ExitStatement, {  // GoToActions

},
{  // Action

})

EXPLORER(AssignStatement, {  // GoToActions

},
{  // Action

})

EXPLORER(PrintStatement, {  // GoToActions

},
{  // Action

})

EXPLORER(ReturnStatement, {  // GoToActions

},
{  // Action

})

EXPLORER(ExpressionStatement, {  // GoToActions

},
{  // Action

})

EXPLORER(EmptyStatement, {  // GoToActions

},
{  // Action

})

EXPLORER(CommaExpressions, {  // GoToActions

},
{  // Action

})

EXPLORER(CommaIdents, {  // GoToActions

},
{  // Action

})

EXPLORER(IdentMemberAccessor, {  // GoToActions

},
{  // Action

})

EXPLORER(IntLiteralMemberAccessor, {  // GoToActions

},
{  // Action

})

EXPLORER(ParenMemberAccessor, {  // GoToActions

},
{  // Action

})

EXPLORER(IndexAccessor, {  // GoToActions

},
{  // Action

})

EXPLORER(Reference, {  // GoToActions

},
{  // Action

})

EXPLORER(Expression, {  // GoToActions

},
{  // Action

})

EXPLORER(XorOperand, {  // GoToActions

},
{  // Action

})

EXPLORER(OrOperand, {  // GoToActions

},
{  // Action

})

EXPLORER(AndOperand, {  // GoToActions

},
{  // Action

})

EXPLORER(Sum, {  // GoToActions

},
{  // Action

})

EXPLORER(Term, {  // GoToActions

},
{  // Action

})

EXPLORER(Unary, {  // GoToActions

},
{  // Action

})

EXPLORER(PrefixOperator, {  // GoToActions

},
{  // Action

})

EXPLORER(TypecheckOperator, {  // GoToActions

},
{  // Action

})

EXPLORER(Call, {  // GoToActions

},
{  // Action

})

EXPLORER(AccessorOperator, {  // GoToActions

},
{  // Action

})

EXPLORER(PrimaryIdent, {  // GoToActions

},
{  // Action

})

EXPLORER(ParenthesesExpression, {  // GoToActions

},
{  // Action

})

EXPLORER(TupleLiteralElement, {  // GoToActions

},
{  // Action

})

EXPLORER(TupleLiteral, {  // GoToActions

},
{  // Action

})

EXPLORER(ShortFuncBody, {  // GoToActions

},
{  // Action

})

EXPLORER(LongFuncBody, {  // GoToActions

},
{  // Action

})

EXPLORER(FuncLiteral, {  // GoToActions

},
{  // Action

})

EXPLORER(TokenLiteral, {  // GoToActions

},
{  // Action

})

EXPLORER(ArrayLiteral, {  // GoToActions

},
{  // Action

})


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

