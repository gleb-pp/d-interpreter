#include "interp/runner.h"
#include "interp/execution.h"
#include "interp/varScopes.h"
#include "interp/input.h"
using namespace std;

namespace interp {

void Run(interp::RuntimeContext& context, ast::Body& program) {
    auto scopes = make_shared<ScopeStack>();
    scopes->Declare(make_shared<Variable>("input", make_shared<interp::InputFunction>()));
    Executor exec(context, scopes);
    program.AcceptVisitor(exec);
}

}
