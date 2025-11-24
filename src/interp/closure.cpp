#include "dinterp/interp/closure.h"

#include "dinterp/interp/execution.h"
#include "dinterp/interp/runtimeContext.h"
#include "dinterp/interp/varScopes.h"
#include "dinterp/syntax.h"
using namespace std;

namespace runtime {

Closure::Closure(const interp::ScopeStack& values, const ast::ClosureDefinition& def)
    : params(def.Params), initialScope(make_shared<interp::ScopeStack>()), code(def.Definition), funcType(def.Type) {
    for (const string& name : def.CapturedExternals) initialScope->Declare(*values.Lookup(name));
}

shared_ptr<RuntimeValue> Closure::UserCall(interp::RuntimeContext& context,
                                           const vector<shared_ptr<RuntimeValue>>& args) const {
    size_t n = params.size();
    if (args.size() != n)
        throw runtime_error("Wrong number arguments supplied to a user call (interpreter's validation is broken)");
    auto scope = make_shared<interp::ScopeStack>(initialScope);
    for (size_t i = 0; i < n; i++) scope->Declare(make_shared<interp::Variable>(params[i], args[i]));
    interp::Executor exec(context, scope);
    auto longBody = dynamic_pointer_cast<ast::LongFuncBody>(code);
    if (longBody) {
        exec.VisitBody(*longBody->funcBody);
        switch (context.State.StateKind()) {
            case interp::RuntimeState::Kind::Throwing:
                return nullptr;
            case interp::RuntimeState::Kind::Running:
                return make_shared<NoneValue>();
            case interp::RuntimeState::Kind::Exiting:
                throw runtime_error("Cannot 'exit' out of a function");
            case interp::RuntimeState::Kind::Returning: {
                auto res = context.State.GetReturnValue();
                context.State = interp::RuntimeState::Running();
                return res;
            }
        }
    }
    dynamic_cast<ast::ShortFuncBody&>(*code).expressionToReturn->AcceptVisitor(exec);
    if (context.State.IsThrowing()) return nullptr;
#ifdef DINTERP_DEBUG
    {
        auto kind = context.State.StateKind();
        if (kind != interp::RuntimeState::Kind::Running)
            throw runtime_error("After evaluation of a short-form function body, the state was " +
                                to_string(static_cast<int>(kind)));
    }
#endif
    return exec.ExpressionValue();
}

shared_ptr<FuncType> Closure::FunctionType() const { return funcType; }

void Closure::DoPrintSelf(ostream& out, [[maybe_unused]] set<shared_ptr<const RuntimeValue>>& recGuard) const {
    out << "<closure: " << funcType->Name() << ">";
}

}  // namespace runtime
