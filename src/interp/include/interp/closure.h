#pragma once
#include "runtime/values.h"
#include "runtimeContext.h"
#include "syntaxext/precomputed.h"
#include "varScopes.h"

namespace runtime {

class Closure : public RuntimeValue {
    std::shared_ptr<interp::ScopeStack> initialScope;
    std::shared_ptr<ast::FuncBody> code;

public:
    Closure(const std::shared_ptr<ast::ClosureDefinition>& def);
    std::shared_ptr<RuntimeValue> UserCall(interp::RuntimeContext& context,
                                           const std::vector<std::shared_ptr<RuntimeValue>>& args) const;
    std::shared_ptr<Type> TypeOfValue() const override;
    void DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>& recGuard) const override;
    virtual ~Closure() override = default;
};

}  // namespace runtime
