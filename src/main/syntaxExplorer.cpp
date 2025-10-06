#include "syntaxExplorer.h"
#include "syntax.h"
#include <sstream>
using namespace std;

static int StrToInt(const std::string& s) {
    istringstream ss(s);
    int res; ss >> res;
    return res;
}

ASTExplorer::ActionCommand::ActionCommand(const string& command, const string& description)
    : Command(command), Description(description) {}

#define EXPLORER_HELP(classname, _gotoactions, _action, literalExplorer)\
class classname##literalExplorer : public ASTExplorer {\
    shared_ptr<ast::classname> node;\
public:\
    classname##literalExplorer(const shared_ptr<ast::classname>& node) : node(node) {}\
    vector<ActionCommand> GetActionCommands() const override _gotoactions\
    optional<shared_ptr<ast::ASTNode>> Action(string command, [[maybe_unused]] ostream& output) const override _action\
    string NodeName() const override { return #classname; }\
    virtual ~classname##literalExplorer() override = default;\
};

#define EXPLORER(classname, _gotoactions, _action)\
    EXPLORER_HELP(classname, _gotoactions, _action, Explorer)

EXPLORER(Body, { // GoToActions
    vector<ActionCommand> res;
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
    vector<ActionCommand> res;
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
    vector<ActionCommand> res({ { "c", "Condition" }, { "t", "Do if true" } });
    if (node->doIfFalse.has_value())
        res.emplace_back("f", "Do if false");
    return res;
},
{  // Action
    switch (command[0]) {
    case 'c':
        return node->condition;
    case 't':
        return node->doIfTrue;
    case 'f':
        return node->doIfFalse;
    }
    return {};
})

EXPLORER(ShortIfStatement, {  // GoToActions
    return vector<ActionCommand>({ { "c", "Condition" }, { "t", "Do if true" } });
},
{  // Action
    if (command[0] == 'c') return node->condition;
    return node->doIfTrue;
})

EXPLORER(WhileStatement, {  // GoToActions
    return vector<ActionCommand>({ { "c", "Condition" }, { "a", "Action" } });
},
{  // Action
    if (command[0] == 'c') return node->condition;
    return node->action;
})

EXPLORER(ForStatement, {  // GoToActions
    vector<ActionCommand> res;
    if (node->optVariableName.has_value())
        res.emplace_back("var", "Cycle variable name");
    if (node->end.has_value()) {
        res.emplace_back("start", "Starting value");
        res.emplace_back("end", "Last value");
    } else
        res.emplace_back("list", "Iterable");
    return res;
},
{  // Action
    if (command == "var") {
        output << node->optVariableName.value()->identifier;
        return {};
    }
    if (command == "start" || command == "list") return node->startOrList;
    return *node->end;
})

EXPLORER(LoopStatement, {  // GoToActions
    return vector<ActionCommand>({ { "b", "Loop body" } });
},
{  // Action
    return node->body;
})

EXPLORER(ExitStatement, {  // GoToActions
    return {};
},
{  // Action
    return {};
})

EXPLORER(AssignStatement, {  // GoToActions
    return vector<ActionCommand>({
        { "d", "Destination (left-hand side)" },
        { "l", "Destination (left-hand side)" },
        { "s", "Source      (right-hand side)" },
        { "r", "Source      (right-hand side)" } });
},
{  // Action
    switch (command[0]) {
    case 'd': case 'l': return node->dest;
    default: return node->src;
    }
})

EXPLORER(PrintStatement, {  // GoToActions
    vector<ActionCommand> res;
    int n = node->expressions.size();
    for (int i = 0; i < n; ++i)
        res.emplace_back(to_string(i), "expressions[" + to_string(i) + "]");
    return res;
},
{  // Action
    return node->expressions[StrToInt(command)];
})

EXPLORER(ReturnStatement, {  // GoToActions
    return vector<ActionCommand>({ { "v", "Return value" } });
},
{  // Action
    return node->returnValue;
})

EXPLORER(ExpressionStatement, {  // GoToActions
    return vector<ActionCommand>({ { "e", "The expression" } });
},
{  // Action
    return node->expr;
})

EXPLORER(EmptyStatement, {  // GoToActions
    return {};
},
{  // Action
    return {};
})

EXPLORER(CommaExpressions, {  // GoToActions
    vector<ActionCommand> res;
    int n = node->expressions.size();
    for (int i = 0; i < n; ++i)
        res.emplace_back(to_string(i), "expressions[" + to_string(i) + "]");
    return res;
},
{  // Action
    return node->expressions[StrToInt(command)];
})

EXPLORER(CommaIdents, {  // GoToActions
    vector<ActionCommand> res;
    int n = node->idents.size();
    for (int i = 0; i < n; ++i)
        res.emplace_back(to_string(i), "idents[" + to_string(i) + "]");
    return res;
},
{  // Action
    output << node->idents[StrToInt(command)]->identifier;
    return {};
})

EXPLORER(IdentMemberAccessor, {  // GoToActions
    return vector<ActionCommand>({ { "i", "The identifier" } });
},
{  // Action
    output << node->name->identifier;
    return {};
})

EXPLORER(IntLiteralMemberAccessor, {  // GoToActions
    return vector<ActionCommand>({ { "i", "The index" } });
},
{  // Action
    output << node->index->value;
    return {};
})

EXPLORER(ParenMemberAccessor, {  // GoToActions
    return vector<ActionCommand>({ { "i", "The index" } });
},
{  // Action
    return node->expr;
})

EXPLORER(IndexAccessor, {  // GoToActions
    return vector<ActionCommand>({ { "i", "The index" } });
},
{  // Action
    return node->expressionInBrackets;
})

EXPLORER(Reference, {  // GoToActions
    vector<ActionCommand> res({ { "b", "The base identifier" } });
    int n = node->accessorChain.size();
    for (int i = 0; i < n; ++i) res.emplace_back("a" + to_string(i), "accessor[" + to_string(i) + "]");
    return res;
},
{  // Action
    bool base = command[0] == 'b';
    if (base) {
        output << node->baseIdent;
        return {};
    } else {
        command.erase(0, 1);
        return node->accessorChain[StrToInt(command)];
    }
})

EXPLORER(Expression, {  // GoToActions
    vector<ActionCommand> res;
    int n = node->operands.size();
    for (int i = 0; i < n; ++i) res.emplace_back(to_string(i), "XOR operands[" + to_string(i) + "]");
    return res;
},
{  // Action
    return node->operands[StrToInt(command)];
})

EXPLORER(OrOperator, {  // GoToActions
    vector<ActionCommand> res;
    int n = node->operands.size();
    for (int i = 0; i < n; ++i) res.emplace_back(to_string(i), "OR operands[" + to_string(i) + "]");
    return res;
},
{  // Action
    return node->operands[StrToInt(command)];
})

EXPLORER(AndOperator, {  // GoToActions
    vector<ActionCommand> res;
    int n = node->operands.size();
    for (int i = 0; i < n; ++i) res.emplace_back(to_string(i), "AND operands[" + to_string(i) + "]");
    return res;
},
{  // Action
    return node->operands[StrToInt(command)];
})

static string to_string(ast::BinaryRelationOperator op) {
    switch (op) {
        case ast::BinaryRelationOperator::Equal: return     "Equal     =";
        case ast::BinaryRelationOperator::Less: return      "Less      <";
        case ast::BinaryRelationOperator::LessEq: return    "LessEq    <=";
        case ast::BinaryRelationOperator::Greater: return   "Greater   >";
        case ast::BinaryRelationOperator::GreaterEq: return "GreaterEq >=";
        case ast::BinaryRelationOperator::NotEqual: return  "NotEqual  /=";
    }
    return "<error>";
}

EXPLORER(BinaryRelation, {  // GoToActions
    vector<ActionCommand> res;
    int n = node->operands.size();
    for (int i = 0; i < n; ++i) {
        res.emplace_back("s" + to_string(i), " operands[" + to_string(i) + "]");
        if (i != n - 1)
            res.emplace_back("o" + to_string(i), "operators[" + to_string(i) +
                             "] (is " + to_string(node->operators[i]) + ")");
    }
    return res;
},
{  // Action
    bool operand = command[0] == 's';
    command.erase(0, 1);
    int index = StrToInt(command);
    if (operand) {
        output << to_string(node->operators[index]);
        return {};
    } else return node->operands[index];
})

static string to_string(ast::Sum::SumOperator op) {
    switch (op) {
        case ast::Sum::SumOperator::Plus: return  "Plus  +";
        case ast::Sum::SumOperator::Minus: return "Minus -";
    }
    return "<error>";
}

EXPLORER(Sum, {  // GoToActions
    vector<ActionCommand> res;
    int n = node->terms.size();
    for (int i = 0; i < n; ++i) {
        res.emplace_back("t" + to_string(i),     "    terms[" + to_string(i) + "]");
        if (i != n - 1)
            res.emplace_back("o" + to_string(i), "operators[" + to_string(i) +
                             "] (is " + to_string(node->operators[i]) + ")");
    }
    return res;
},
{  // Action
    bool operand = command[0] == 't';
    command.erase(0, 1);
    int index = StrToInt(command);
    if (operand) {
        output << to_string(node->operators[index]);
        return {};
    } else return node->terms[index];
})

static string to_string(ast::Term::TermOperator op) {
    switch (op) {
        case ast::Term::TermOperator::Times: return  "Times  *";
        case ast::Term::TermOperator::Divide: return "Divide /";
    }
    return "<error>";
}

EXPLORER(Term, {  // GoToActions
    vector<ActionCommand> res;
    int n = node->unaries.size();
    for (int i = 0; i < n; ++i) {
        res.emplace_back("u" + to_string(i),     "  unaries[" + to_string(i) + "]");
        if (i != n - 1)
            res.emplace_back("o" + to_string(i), "operators[" + to_string(i) +
                             "] (is " + to_string(node->operators[i]) + ")");
    }
    return res;
},
{  // Action
    bool operand = command[0] == 'u';
    command.erase(0, 1);
    int index = StrToInt(command);
    if (operand) {
        output << to_string(node->operators[index]);
        return {};
    } else return node->unaries[index];
})

static string to_string(ast::PrefixOperator::PrefixOperatorKind op) {
    switch (op) {
        case ast::PrefixOperator::PrefixOperatorKind::Plus: return  "Plus  +";
        case ast::PrefixOperator::PrefixOperatorKind::Minus: return "Minus -";
        case ast::PrefixOperator::PrefixOperatorKind::Not: return   "Not";
    }
    return "<error>";
}

EXPLORER(Unary, {  // GoToActions
    vector<ActionCommand> res;
    int i = 0;
    for (const auto& pref : node->prefixOps) {
        res.emplace_back("e" + to_string(i), "prefixOperators[" + to_string(i) +
                         "] (is " + to_string(pref->kind) + ")");
        ++i;
    }
    res.emplace_back("p", "The primary expression");
    int n = node->postfixOps.size();
    for (int i = 0; i < n; ++i) res.emplace_back("o" + to_string(i), "postfixOperators[" + to_string(i) + "]");
    return res;
},
{  // Action
    char kind = command[0];
    command.erase(0, 1);
    int index = command.empty() ? 0 : StrToInt(command);
    switch (kind) {
        case 'e': return node->prefixOps[index];
        case 'o': return node->postfixOps[index];
        default: return node->expr;
    }
})

EXPLORER(PrefixOperator, {  // GoToActions
    return vector<ActionCommand>({ { "?", "The precedence of the operator" } });
},
{  // Action
    output << node->precedence();
    return {};
})

static string to_string(ast::TypeId id) {
    switch (id) {
case ast::TypeId::
    }
}

EXPLORER(TypecheckOperator, {  // GoToActions
    return 
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
VISITOR(OrOperator)
VISITOR(AndOperator)
VISITOR(BinaryRelation)
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

