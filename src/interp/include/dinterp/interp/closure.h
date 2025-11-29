#pragma once
#include "dinterp/runtime/values.h"
#include "dinterp/syntaxext/precomputed.h"
#include "runtimeContext.h"
#include "userCallable.h"
#include "varScopes.h"

namespace dinterp {
namespace runtime {

class Closure : public interp::UserCallable {
    std::vector<std::string> params;
    std::shared_ptr<interp::ScopeStack> initialScope;
    std::shared_ptr<ast::FuncBody> code;
    std::shared_ptr<runtime::FuncType> funcType;

public:
    Closure(const interp::ScopeStack& values, const ast::ClosureDefinition& def);
    std::shared_ptr<RuntimeValue> UserCall(interp::RuntimeContext& context,
                                           const std::vector<std::shared_ptr<RuntimeValue>>& args) const override;
    std::shared_ptr<FuncType> FunctionType() const override;
    void DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>& recGuard) const override;
    virtual ~Closure() override = default;
};

}  // namespace runtime
}  // namespace dinterp
