#pragma once
#include <memory>
#include <variant>

#include "dinterp/locators/locator.h"
#include "dinterp/runtime.h"

namespace dinterp {
namespace semantic {

struct ScopeStats {
    std::vector<std::pair<std::string, locators::SpanLocator>> uselessAssignments;
    std::vector<std::pair<std::string, locators::SpanLocator>> variablesNeverUsed;
    std::map<std::string, bool> referencedExternals;  // true if assigned
};

class ValueTimeline {
    struct Var {
        runtime::TypeOrValue val;
        std::set<std::shared_ptr<locators::SpanLocator>> lastUnusedAssignments;
        locators::SpanLocator declaration;
        bool used = false;
        Var(const locators::SpanLocator& declarationloc);
    };

    struct Scope {
        std::map<std::string, Var> vars;
        std::map<std::string, bool> externalReferences;  // true if assigned
    };

    std::vector<Scope> stack;
    std::vector<size_t> blindScopeIndices;

    std::optional<std::pair<size_t, const Var*>> Lookup(const std::string& name) const;
    std::optional<std::pair<size_t, Var*>> Lookup(const std::string& name);

public:
    std::optional<runtime::TypeOrValue> LookupVariable(const std::string& name);
    void MakeAllUnknown();
    // Normal scope
    void StartScope();
    // Blind Scope: from inside, all external variables are of type UnknownType
    void StartBlindScope();
    ScopeStats EndScope();
    bool AssignType(const std::string& name, const std::shared_ptr<runtime::Type>& type, locators::SpanLocator pos);
    bool AssignValue(const std::string& name, const std::shared_ptr<runtime::RuntimeValue>& precomputed,
                     locators::SpanLocator pos);
    bool Assign(const std::string& name, const runtime::TypeOrValue& precomputed, locators::SpanLocator pos);
    bool AssignUnknownButUsed(const std::string& name);
    bool Declare(const std::string& name, locators::SpanLocator pos);
    locators::SpanLocator LookupDeclaration(const std::string& name);
    void MergeTimelines(const ValueTimeline& other);  // Use after an If statement
};

}  // namespace semantic
}
