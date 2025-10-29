#pragma once
#include <memory>
#include <variant>

#include "runtime/types.h"
#include "runtime/values.h"

class ValueTimeline {
public:
    std::optional<std::variant<std::shared_ptr<runtime::Type>, std::shared_ptr<runtime::RuntimeValue>>> LookupVariable()
        const;
    void MakeAllUnknown();
    // Normal scope
    void StartScope();
    // Blind Scope: from inside, all variables are of type UnknownType
    void StartBlindScope();
    // EndScope: returns a list of all modified external variables
    std::vector<std::string> EndScope();
    void Assign(const std::string& name, const std::shared_ptr<runtime::Type>& type);
    void Assign(const std::string& name, const std::shared_ptr<runtime::RuntimeValue>& precomputed);
    void MergeTimelines(const ValueTimeline& other);  // Use after an If statement
};
