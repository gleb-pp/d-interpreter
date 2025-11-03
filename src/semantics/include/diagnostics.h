#pragma once
#include <string>

#include "complog/CompilationMessage.h"
#include "locators/locator.h"
#include "runtime/types.h"

namespace semantic_errors {

class SpanLocatorMessage : public complog::CompilationMessage {
protected:
    locators::SpanLocator loc;

public:
    SpanLocatorMessage(complog::Severity severity, const std::string& code, locators::SpanLocator pos);
    std::vector<locators::Locator> Locators() const override;
    std::vector<locators::SpanLocator> SpanLocators() const override;
    virtual ~SpanLocatorMessage() override = default;
};

class VariableNotDefined : public SpanLocatorMessage {
    std::string varName;

public:
    VariableNotDefined(const locators::SpanLocator pos, const std::string& varName);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~VariableNotDefined() override = default;
};

using VectorOfSpanTypes = std::vector<std::pair<locators::SpanLocator, std::shared_ptr<runtime::Type>>>;

class OperatorNotApplicable : public complog::CompilationMessage {
    std::string opName;
    VectorOfSpanTypes types;

public:
    OperatorNotApplicable(const std::string& peratorName, const VectorOfSpanTypes& operands);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    std::vector<locators::Locator> Locators() const override;
    std::vector<locators::SpanLocator> SpanLocators() const override;
    virtual ~OperatorNotApplicable() override = default;
};

class CodeUnreachable : public SpanLocatorMessage {
    bool removed;

public:
    CodeUnreachable(locators::SpanLocator pos, bool removed);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~CodeUnreachable() override = default;
};

class IfConditionAlwaysKnown : public SpanLocatorMessage {
    bool conditionValue;

public:
    IfConditionAlwaysKnown(bool value, locators::SpanLocator pos);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~IfConditionAlwaysKnown() override = default;
};

class WhileConditionFalseAtStart : public SpanLocatorMessage {
public:
    WhileConditionFalseAtStart(locators::SpanLocator pos);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~WhileConditionFalseAtStart() override = default;
};

class ExpressionStatementNoSideEffects : public SpanLocatorMessage {
public:
    ExpressionStatementNoSideEffects(locators::SpanLocator pos);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~ExpressionStatementNoSideEffects() override = default;
};

class EvaluationException : public SpanLocatorMessage {
    std::string msg;

public:
    EvaluationException(locators::SpanLocator pos, const std::string& message);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~EvaluationException() override = default;
};

class CannotAssignNamedFieldInTuple : public SpanLocatorMessage {
    std::string fieldname;

public:
    CannotAssignNamedFieldInTuple(locators::SpanLocator pos, const std::string& fieldName);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~CannotAssignNamedFieldInTuple() override = default;
};

class FieldsOnlyAssignableInTuples : public SpanLocatorMessage {
    std::shared_ptr<runtime::Type> type;

public:
    FieldsOnlyAssignableInTuples(locators::SpanLocator pos, const std::shared_ptr<runtime::Type>& triedToAssignIn);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~FieldsOnlyAssignableInTuples() override = default;
};

class CannotAssignIndexedFieldInTuple : public SpanLocatorMessage {
public:
    CannotAssignIndexedFieldInTuple(locators::SpanLocator pos);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~CannotAssignIndexedFieldInTuple() override = default;
};

class SubscriptAssignmentOnlyInArrays : public SpanLocatorMessage {
    std::shared_ptr<runtime::Type> type;

public:
    SubscriptAssignmentOnlyInArrays(locators::SpanLocator pos, const std::shared_ptr<runtime::Type>& triedType);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~SubscriptAssignmentOnlyInArrays() override = default;
};

class BadSubscriptIndexType : public SpanLocatorMessage {
    std::shared_ptr<runtime::Type> type;

public:
    BadSubscriptIndexType(locators::SpanLocator pos, const std::shared_ptr<runtime::Type>& triedType);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~BadSubscriptIndexType() override = default;
};

class IntegerZeroDivisionWarning : public SpanLocatorMessage {
public:
    IntegerZeroDivisionWarning(locators::SpanLocator pos);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~IntegerZeroDivisionWarning() override = default;
};

}  // namespace semantic_errors
