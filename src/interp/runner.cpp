#include "dinterp/interp/runner.h"

#include "dinterp/interp/execution.h"
#include "dinterp/interp/input.h"
#include "dinterp/interp/varScopes.h"
using namespace std;

namespace interp {

void Run(interp::RuntimeContext& context, ast::Body& program) {
    auto scopes = make_shared<ScopeStack>();
    scopes->Declare(make_shared<Variable>("input", make_shared<interp::InputFunction>()));
    Executor exec(context, scopes);
    program.AcceptVisitor(exec);
}

}  // namespace interp
