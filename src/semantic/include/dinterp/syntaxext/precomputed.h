#pragma once
#include "dinterp/locators/locator.h"
#include "dinterp/runtime/types.h"
#include "dinterp/runtime/values.h"
#include "dinterp/syntax.h"

namespace dinterp {
namespace ast {

class PrecomputedValue : public Expression {
public:
    std::shared_ptr<runtime::RuntimeValue> Value;
    PrecomputedValue(const locators::SpanLocator& pos, const std::shared_ptr<runtime::RuntimeValue>& val);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~PrecomputedValue() override = default;
};

class ClosureDefinition : public Expression {
public:
    std::shared_ptr<runtime::FuncType> Type;
    std::shared_ptr<FuncBody> Definition;
    std::vector<std::string> Params;
    std::vector<std::string> CapturedExternals;
    ClosureDefinition(const locators::SpanLocator& pos, const std::shared_ptr<runtime::FuncType>& type,
                      const std::shared_ptr<FuncBody>& definition, const std::vector<std::string>& params,
                      const std::vector<std::string>& capturedExternals);
    void AcceptVisitor(IASTVisitor& vis) override;
    virtual ~ClosureDefinition() override = default;
};

}  // namespace ast
}  // namespace dinterp
