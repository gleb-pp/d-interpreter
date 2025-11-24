#pragma once
#include "runtime.h"
#include "runtime/values.h"
#include "runtimeContext.h"

namespace interp {

class UserCallable : public runtime::RuntimeValue {
public:
    virtual std::shared_ptr<runtime::RuntimeValue> UserCall(
        RuntimeContext& context, const std::vector<std::shared_ptr<runtime::RuntimeValue>>& args) const = 0;
    std::shared_ptr<runtime::Type> TypeOfValue() const override;
    virtual std::shared_ptr<runtime::FuncType> FunctionType() const = 0;
    virtual ~UserCallable() = default;
};

}  // namespace interp
