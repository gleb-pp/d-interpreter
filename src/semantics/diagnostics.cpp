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

ConditionMustBeBoolean::ConditionMustBeBoolean(const locators::SpanLocator pos,
                                               const shared_ptr<runtime::Type>& gotType)
    : SpanLocatorMessage(complog::Severity::Error(), "ConditionMustBeBoolean", pos), received(gotType) {}
void ConditionMustBeBoolean::WriteMessageToStream(ostream& out,
                                                  const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": expected the condition to be a boolean, but found \"" << received->Name() << "\".\n";
}

VariableNotDefined::VariableNotDefined(const locators::SpanLocator pos, const string& varName)
    : SpanLocatorMessage(complog::Severity::Error(), "VariableNotDefined", pos), varName(varName) {}
void VariableNotDefined::WriteMessageToStream(ostream& out,
                                              const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": Variable \"" << varName << "\" was not declared.\n";
}

VariableRedefined::VariableRedefined(const locators::SpanLocator pos, const string& varName)
    : SpanLocatorMessage(complog::Severity::Error(), "VariableRedefined", pos), varName(varName) {}
void VariableRedefined::WriteMessageToStream(ostream& out,
                                              const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": Variable \"" << varName << "\" has already been declared in this scope.\n";
}

OperatorNotApplicable::OperatorNotApplicable(const string& operatorName, const VectorOfSpanTypes& operands)
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
    out << " - at: \n";
    size_t n = types.size();
    for (size_t i = 0; i < n; i++)
        out << types[i].first.Pretty() << (i + 1 < n ? ';' : '.') << '\n';
}
vector<locators::Locator> OperatorNotApplicable::Locators() const { return {}; }
vector<locators::SpanLocator> OperatorNotApplicable::SpanLocators() const {
    vector<locators::SpanLocator> res;
    ranges::transform(
        types, back_inserter(res),
        [](const pair<locators::SpanLocator, shared_ptr<runtime::Type>>& kv) { return kv.first; });
    return res;
}

CodeUnreachable::CodeUnreachable(locators::SpanLocator pos, bool removed)
    : SpanLocatorMessage(complog::Severity::Warning(), "CodeUnreachable", pos), removed(removed) {}
void CodeUnreachable::WriteMessageToStream(ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": Code unreachable";
    if (removed) out << " (removed)";
    out << ".\n";
}

IfConditionAlwaysKnown::IfConditionAlwaysKnown(bool value, locators::SpanLocator pos)
    : SpanLocatorMessage(complog::Severity::Warning(), "IfConditionAlwaysKnown", pos), conditionValue(value) {}
void IfConditionAlwaysKnown::WriteMessageToStream(ostream& out,
                                                  const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": Branching operator's condition is always " << (conditionValue ? "true" : "false") << ".\n";
}

WhileConditionFalseAtStart::WhileConditionFalseAtStart(locators::SpanLocator pos)
    : SpanLocatorMessage(complog::Severity::Warning(), "WhileConditionFalseAtStart", pos) {}
void WhileConditionFalseAtStart::WriteMessageToStream(ostream& out,
                                                      const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": While cycle's condition is known to be false at start.\n";
}

WhileConditionNotBoolAtStart::WhileConditionNotBoolAtStart(locators::SpanLocator pos, const shared_ptr<runtime::Type>& received)
    : SpanLocatorMessage(complog::Severity::Warning(), "WhileConditionNotBoolAtStart", pos), received(received) {}
void WhileConditionNotBoolAtStart::WriteMessageToStream(ostream& out,
                                                      const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": While cycle's condition is known to not be a boolean at start (is a \"" << received->Name() << "\").\n";
}

ExpressionStatementNoSideEffects::ExpressionStatementNoSideEffects(locators::SpanLocator pos)
    : SpanLocatorMessage(complog::Severity::Warning(), "ExpressionStatementNoSideEffects", pos) {}
void ExpressionStatementNoSideEffects::WriteMessageToStream(
    ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": The expression has no side effects; optimized.\n";
}

EvaluationException::EvaluationException(locators::SpanLocator pos, const string& message)
    : SpanLocatorMessage(complog::Severity::Error(), "EvaluationException", pos), msg(message) {}
void EvaluationException::WriteMessageToStream(ostream& out,
                                               const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": There was an exception while evaluating the expression: " << msg << "\n";
}

NoSuchField::NoSuchField(locators::SpanLocator pos, const shared_ptr<runtime::Type>& type, const string& fieldName)
    : SpanLocatorMessage(complog::Severity::Error(), "NoSuchField", pos), type(type), fieldname(fieldName) {}
void NoSuchField::WriteMessageToStream(ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": Object of type \"" << type->Name() << "\" had no field \"" << fieldname << "\".\n";
}

CannotAssignNamedFieldInTuple::CannotAssignNamedFieldInTuple(locators::SpanLocator pos, const string& fieldName)
    : SpanLocatorMessage(complog::Severity::Error(), "CannotAssignNamedFieldInTuple", pos), fieldname(fieldName) {}
void CannotAssignNamedFieldInTuple::WriteMessageToStream(ostream& out,
                                                         const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": The tuple contains no such named field: \"" << fieldname << "\".\n";
}

FieldsOnlyAssignableInTuples::FieldsOnlyAssignableInTuples(locators::SpanLocator pos,
                                                           const shared_ptr<runtime::Type>& triedToAssignIn)
    : SpanLocatorMessage(complog::Severity::Error(), "FieldsOnlyAssignableInTuples", pos), type(triedToAssignIn) {}
void FieldsOnlyAssignableInTuples::WriteMessageToStream(ostream& out,
                                                        const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": Can only assign to fields in tuples, but tried to in \"" << type->Name() << "\".\n";
}

CannotAssignIndexedFieldInTuple::CannotAssignIndexedFieldInTuple(locators::SpanLocator pos, const string& intRepr)
    : SpanLocatorMessage(complog::Severity::Error(), "CannotAssignIndexedFieldInTuple", pos), intRepr(intRepr) {}
void CannotAssignIndexedFieldInTuple::WriteMessageToStream(
    ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": Tuple index \"" << intRepr << "\" is out of range.\n";
}

SubscriptAssignmentOnlyInArrays::SubscriptAssignmentOnlyInArrays(locators::SpanLocator pos,
                                                                 const shared_ptr<runtime::Type>& triedType)
    : SpanLocatorMessage(complog::Severity::Error(), "SubscriptAssignmentOnlyInArrays", pos), type(triedType) {}
void SubscriptAssignmentOnlyInArrays::WriteMessageToStream(
    ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": Can only assign elements of arrays, but provided \"" << type->Name() << "\".\n";
}

BadSubscriptIndexType::BadSubscriptIndexType(locators::SpanLocator pos, const shared_ptr<runtime::Type>& triedType)
    : SpanLocatorMessage(complog::Severity::Error(), "BadSubscriptIndexType", pos), type(triedType) {}
void BadSubscriptIndexType::WriteMessageToStream(ostream& out,
                                                 const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": Cannot use \"" << type->Name() << "\" as index in this subscript.\n";
}

IntegerZeroDivisionWarning::IntegerZeroDivisionWarning(locators::SpanLocator pos)
    : SpanLocatorMessage(complog::Severity::Warning(), "IntegerZeroDivisionWarning", pos) {}
void IntegerZeroDivisionWarning::WriteMessageToStream(ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": Looks like integer division by zero; this will crash the program during execution.\n";
}

TriedToCallNonFunction::TriedToCallNonFunction(locators::SpanLocator pos, const shared_ptr<runtime::Type>& type)
    : SpanLocatorMessage(complog::Severity::Error(), "TriedToCallNonFunction", pos), type(type) {}
void TriedToCallNonFunction::WriteMessageToStream(ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": Attempted to call a value of type \"" << type->Name() << "\" (only function calls are allowed).\n";
}

WrongArgumentCount::WrongArgumentCount(locators::SpanLocator pos, size_t expected, size_t given)
    : SpanLocatorMessage(complog::Severity::Error(), "WrongArgumentCount", pos), expected(expected), given(given) {}
void WrongArgumentCount::WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": This function expects " << expected << " arguments, but " << given << " were provided.\n";
}

WrongArgumentType::WrongArgumentType(locators::SpanLocator pos, const std::shared_ptr<runtime::Type>& expected,
                  const std::shared_ptr<runtime::Type>& given)
    : SpanLocatorMessage(complog::Severity::Error(), "WrongArgumentCount", pos), expected(expected), given(given) {}
void WrongArgumentType::WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << loc.Pretty() << ": Expected an argument of type \"" << expected->Name() << "\", but received \"" << given->Name() << "\".\n";
}

DuplicateFieldNames::DuplicateFieldNames(const std::string& name, const std::vector<locators::SpanLocator>& positions)
    : complog::CompilationMessage(complog::Severity::Error(), "DuplicateFieldNames"), name(name), positions(positions) {}
void DuplicateFieldNames::WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << "Duplicate field name \"" << name << "\" at \n";
    bool first = true;
    for (auto& loc : positions) {
        if (!first) out << ";\n";
        first = false;
        out << loc.Pretty();
    }
    out << ".\n";
}
std::vector<locators::Locator> DuplicateFieldNames::Locators() const { return {}; }
std::vector<locators::SpanLocator> DuplicateFieldNames::SpanLocators() const {
    return positions;
}

DuplicateParameterNames::DuplicateParameterNames(const std::string& name, const std::vector<locators::SpanLocator>& positions)
    : complog::CompilationMessage(complog::Severity::Error(), "DuplicateParameterNames"), name(name), positions(positions) {}
void DuplicateParameterNames::WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << "Duplicate parameter name \"" << name << "\" at \n";
    bool first = true;
    for (auto& loc : positions) {
        if (!first) out << ";\n";
        first = false;
        out << loc.Pretty();
    }
    out << ".\n";
}
std::vector<locators::Locator> DuplicateParameterNames::Locators() const { return {}; }
std::vector<locators::SpanLocator> DuplicateParameterNames::SpanLocators() const {
    return positions;
}

AssignedValueUnused::AssignedValueUnused(const locators::SpanLocator pos, const std::string& varName)
    : SpanLocatorMessage(complog::Severity::Warning(), "AssignedValueUnused", pos), varName(varName) {}
void AssignedValueUnused::WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << "The value assigned to \"" << varName << "\" at " << loc.Pretty() << " is never accessed.\n";
}

VariableNeverUsed::VariableNeverUsed(const locators::SpanLocator pos, const std::string& varName)
    : SpanLocatorMessage(complog::Severity::Warning(), "VariableNeverUsed", pos), varName(varName) {}
void VariableNeverUsed::WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << "Variable \"" << varName << "\" declared at " << loc.Pretty() << " but never used.\n";
}

NoneValueAccessed::NoneValueAccessed(const locators::SpanLocator pos, const std::string& varName)
    : SpanLocatorMessage(complog::Severity::Warning(), "NoneValueAccessed", pos), varName(varName) {}
void NoneValueAccessed::WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const {
    out << "The variable \"" << varName << "\" probably contains no value at " << loc.Pretty() << ".\n";
}

}  // namespace semantic_errors
