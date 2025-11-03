#include "diagnostics.h"

#include <algorithm>
#include <iterator>

#include "complog/CompilationMessage.h"
using namespace std;

namespace semantic_errors {

SpanLocatorMessage::SpanLocatorMessage(complog::Severity severity, const string& code, locators::SpanLocator pos)
    : complog::CompilationMessage(severity, code), loc(pos) {}
vector<locators::Locator> SpanLocatorMessage::Locators() const { return {}; }
vector<locators::SpanLocator> SpanLocatorMessage::SpanLocators() const { return {loc}; }

VariableNotDefined::VariableNotDefined(const locators::SpanLocator pos, const string& varName)
    : SpanLocatorMessage(complog::Severity::Error(), "VariableNotDefined", pos), varName(varName) {}
void VariableNotDefined::WriteMessageToStream(ostream& out,
                                              const complog::CompilationMessage::FormatOptions& opts) const {
    out << "Variable \"" << varName << "\" was not declared.\n";
}

OperatorNotApplicable::OperatorNotApplicable(
    const string& operatorName, const vector<pair<locators::SpanLocator, shared_ptr<runtime::Type>>>& operands)
    : complog::CompilationMessage(complog::Severity::Error(), "OperatorNotApplicable"),
      opName(operatorName),
      types(operands) {}
void OperatorNotApplicable::WriteMessageToStream(ostream& out,
                                                 const complog::CompilationMessage::FormatOptions& opts) const {
    out << "The operator \"" << opName << "\" cannot be applied to type(s) ";
    bool first = true;
    for (auto& kv : types) {
        if (!first) out << ", ";
        first = false;
        out << '"' << kv.second->Name() << '"';
    }
    out << ".\n";
}
vector<locators::Locator> OperatorNotApplicable::Locators() const { return {}; }
vector<locators::SpanLocator> OperatorNotApplicable::SpanLocators() const {
    vector<locators::SpanLocator> res;
    std::ranges::transform(
        types, back_inserter(res),
        [](const std::pair<locators::SpanLocator, shared_ptr<runtime::Type>>& kv) { return kv.first; });
    return res;
}

CodeUnreachable::CodeUnreachable(locators::SpanLocator pos, bool removed)
    : SpanLocatorMessage(complog::Severity::Warning(), "CodeUnreachable", pos), removed(removed) {}
void CodeUnreachable::WriteMessageToStream(ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << "Code unreachable";
    if (removed) out << " (removed)";
    out << ".\n";
}

IfConditionAlwaysKnown::IfConditionAlwaysKnown(bool value, locators::SpanLocator pos)
    : SpanLocatorMessage(complog::Severity::Warning(), "IfConditionAlwaysKnown", pos), conditionValue(value) {}
void IfConditionAlwaysKnown::WriteMessageToStream(ostream& out,
                                                  const complog::CompilationMessage::FormatOptions& opts) const {
    out << "Branching operator's condition is always " << (conditionValue ? "true" : "false") << ".\n";
}

WhileConditionFalseAtStart::WhileConditionFalseAtStart(locators::SpanLocator pos)
    : SpanLocatorMessage(complog::Severity::Warning(), "WhileConditionFalseAtStart", pos) {}
void WhileConditionFalseAtStart::WriteMessageToStream(ostream& out,
                                                      const complog::CompilationMessage::FormatOptions& opts) const {
    out << "While cycle's condition is known to be false at start.\n";
}

ExpressionStatementNoSideEffects::ExpressionStatementNoSideEffects(locators::SpanLocator pos)
    : SpanLocatorMessage(complog::Severity::Warning(), "ExpressionStatementNoSideEffects", pos) {}
void ExpressionStatementNoSideEffects::WriteMessageToStream(
    ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << "The expression has no side effects; optimized.\n";
}

EvaluationException::EvaluationException(locators::SpanLocator pos, const string& message)
    : SpanLocatorMessage(complog::Severity::Error(), "EvaluationException", pos), msg(message) {}
void EvaluationException::WriteMessageToStream(ostream& out,
                                               const complog::CompilationMessage::FormatOptions& opts) const {
    out << "There was an exception while evaluating the expression: " << msg << "\n";
}

CannotAssignNamedFieldInTuple::CannotAssignNamedFieldInTuple(locators::SpanLocator pos, const string& fieldName)
    : SpanLocatorMessage(complog::Severity::Error(), "CannotAssignNamedFieldInTuple", pos), fieldname(fieldName) {}
void CannotAssignNamedFieldInTuple::WriteMessageToStream(ostream& out,
                                                         const complog::CompilationMessage::FormatOptions& opts) const {
    out << "The tuple contains no such named field: \"" << fieldname << "\".\n";
}

FieldsOnlyAssignableInTuples::FieldsOnlyAssignableInTuples(locators::SpanLocator pos,
                                                           const shared_ptr<runtime::Type>& triedToAssignIn)
    : SpanLocatorMessage(complog::Severity::Error(), "FieldsOnlyAssignableInTuples", pos), type(triedToAssignIn) {}
void FieldsOnlyAssignableInTuples::WriteMessageToStream(ostream& out,
                                                        const complog::CompilationMessage::FormatOptions& opts) const {
    out << "Can only assign to fields in tuples, but tried to in \"" << type->Name() << "\".\n";
}

CannotAssignIndexedFieldInTuple::CannotAssignIndexedFieldInTuple(locators::SpanLocator pos)
    : SpanLocatorMessage(complog::Severity::Error(), "CannotAssignIndexedFieldInTuple", pos) {}
void CannotAssignIndexedFieldInTuple::WriteMessageToStream(
    ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << "Tuple index out of range.\n";
}

SubscriptAssignmentOnlyInArrays::SubscriptAssignmentOnlyInArrays(locators::SpanLocator pos,
                                                                 const shared_ptr<runtime::Type>& triedType)
    : SpanLocatorMessage(complog::Severity::Error(), "SubscriptAssignmentOnlyInArrays", pos), type(triedType) {}
void SubscriptAssignmentOnlyInArrays::WriteMessageToStream(
    ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << "Can only assign elements of arrays, but provided \"" << type->Name() << "\".\n";
}

BadSubscriptIndexType::BadSubscriptIndexType(locators::SpanLocator pos, const std::shared_ptr<runtime::Type>& triedType)
    : SpanLocatorMessage(complog::Severity::Error(), "BadSubscriptIndexType", pos), type(triedType) {}
void BadSubscriptIndexType::WriteMessageToStream(std::ostream& out,
                                                 const complog::CompilationMessage::FormatOptions& opts) const {
    out << "Cannot use \"" << type->Name() << "\" as index in this subscript.\n";
}

}  // namespace semantic_errors
