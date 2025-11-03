#include "precomputed.h"
using namespace std;

namespace ast {

PrecomputedValue::PrecomputedValue(const locators::SpanLocator& pos, const std::shared_ptr<runtime::RuntimeValue>& val)
    : Expression(pos), Value(val) {}
void PrecomputedValue::AcceptVisitor(IASTVisitor& vis) { vis.VisitCustom(*this); }

}  // namespace ast
