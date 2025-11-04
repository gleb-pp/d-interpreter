#pragma once
#include <memory>
#include <variant>

#include "locators/locator.h"
#include "runtime/types.h"
#include "runtime/values.h"

struct ScopeStats {
    std::vector<std::pair<std::string, locators::SpanLocator>> uselessAssignments;
    std::vector<std::pair<std::string, locators::SpanLocator>> variablesNeverUsed;
    std::vector<std::string> assignedExternals;
};

class ValueTimeline {
public:
    std::optional<std::variant<std::shared_ptr<runtime::Type>, std::shared_ptr<runtime::RuntimeValue>>> LookupVariable(
        const std::string& name) const;
    void MakeAllUnknown();
    // Normal scope
    void StartScope();
    // Blind Scope: from inside, all variables are of type UnknownType
    void StartBlindScope();
    ScopeStats EndScope();
    void Assign(const std::string& name, const std::shared_ptr<runtime::Type>& type, locators::SpanLocator pos);
    void Assign(const std::string& name, const std::shared_ptr<runtime::RuntimeValue>& precomputed, locators::SpanLocator pos);
    bool Declare(const std::string& name, locators::SpanLocator pos);
    void MergeTimelines(const ValueTimeline& other);  // Use after an If statement
};
