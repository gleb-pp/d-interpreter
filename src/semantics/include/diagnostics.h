#pragma once
#include <initializer_list>
#include <string>
#include "complog/CompilationMessage.h"
#include "locators/locator.h"
#include "runtime/types.h"

namespace semantic_errors {

class SingleLocatorMessage : public complog::CompilationMessage {
protected:
    locators::Locator loc;
public:
    SingleLocatorMessage(complog::Severity severity, const std::string& code, locators::Locator pos);
    std::vector<locators::Locator> Locators() const override;
    virtual ~SingleLocatorMessage() override = default;
};

class VariableNotDefined : public SingleLocatorMessage {
    std::string varName;
public:
    VariableNotDefined(const locators::Locator pos, const std::string& varName);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~VariableNotDefined() override = default;
};

class OperatorNotApplicable : public complog::CompilationMessage {
    std::string opName;
    std::vector<std::pair<locators::Locator, std::shared_ptr<runtime::Type>>> types;
public:
    OperatorNotApplicable(const std::string& operatorName,
                          std::initializer_list<std::pair<locators::Locator, std::shared_ptr<runtime::Type>>> operands);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    std::vector<locators::Locator> Locators() const override;
    virtual ~OperatorNotApplicable() override = default;
};

class ArgumentCountWrong : public SingleLocatorMessage {
    std::shared_ptr<runtime::FuncType> func;
    size_t providedCount;
public:
    ArgumentCountWrong(const std::shared_ptr<runtime::FuncType>& function, size_t providedCount, locators::Locator pos);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~ArgumentCountWrong() override = default;
};

class ArgumentTypeMismatch : public SingleLocatorMessage {
    std::shared_ptr<runtime::Type> expected, provided;
public:
    ArgumentTypeMismatch(const std::shared_ptr<runtime::Type>& expected, const std::shared_ptr<runtime::Type>& provided,
                         locators::Locator pos);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~ArgumentTypeMismatch() override = default;
};

class CodeUnreachable : public SingleLocatorMessage {
    bool removed;
public:
    CodeUnreachable(locators::Locator pos, bool removed);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~CodeUnreachable() override = default;
};

class IfConditionAlwaysKnown : public SingleLocatorMessage {
    bool conditionValue;
public:
    IfConditionAlwaysKnown(bool value, locators::Locator ifStatementPos);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~IfConditionAlwaysKnown() override = default;
};

class WhileConditionKnownAtStart : public SingleLocatorMessage {
    bool conditionValue;
public:
    WhileConditionKnownAtStart(bool value, locators::Locator ifStatementPos);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~WhileConditionKnownAtStart() override = default;
};

class ExpressionStatementNoSideEffects : public SingleLocatorMessage {
public:
    ExpressionStatementNoSideEffects(locators::Locator pos);
    void WriteMessageToStream(std::ostream& out, const complog::CompilationMessage::FormatOptions& opts) const override;
    virtual ~ExpressionStatementNoSideEffects() override = default;
};

}
