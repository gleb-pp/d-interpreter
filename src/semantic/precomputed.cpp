#include "dinterp/syntaxext/precomputed.h"
using namespace std;

namespace ast {

PrecomputedValue::PrecomputedValue(const locators::SpanLocator& pos, const std::shared_ptr<runtime::RuntimeValue>& val)
    : Expression(pos), Value(val) {}
void PrecomputedValue::AcceptVisitor(IASTVisitor& vis) { vis.VisitCustom(*this); }

ClosureDefinition::ClosureDefinition(const locators::SpanLocator& pos, const std::shared_ptr<runtime::FuncType>& type,
                                     const std::shared_ptr<FuncBody>& definition,
                                     const std::vector<std::string>& params,
                                     const std::vector<std::string>& capturedExternals)
    : Expression(pos), Type(type), Definition(definition), Params(params), CapturedExternals(capturedExternals) {}
void ClosureDefinition::AcceptVisitor(IASTVisitor& vis) { vis.VisitCustom(*this); }

}  // namespace ast
