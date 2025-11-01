#pragma once
#include "locators/locator.h"
#include "runtime/types.h"
#include "runtime/values.h"
#include "syntax.h"

namespace ast {

class PrecomputedValue : public Expression {
public:
    std::shared_ptr<runtime::RuntimeValue> Value;
    PrecomputedValue(const locators::SpanLocator& pos, const std::shared_ptr<runtime::RuntimeValue>& val);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~PrecomputedValue() override = default;
};

}  // namespace ast
