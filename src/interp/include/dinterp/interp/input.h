#pragma once
#include "dinterp/runtime.h"
#include "userCallable.h"

namespace interp {

class InputFunction : public UserCallable {
public:
    std::shared_ptr<runtime::RuntimeValue> UserCall(
        RuntimeContext& context, const std::vector<std::shared_ptr<runtime::RuntimeValue>>& args) const override;
    std::shared_ptr<runtime::FuncType> FunctionType() const override;
    void DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>& recGuard) const override;
    virtual ~InputFunction() override = default;
};

}  // namespace interp
