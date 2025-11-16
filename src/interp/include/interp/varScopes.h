#pragma once
#include <map>
#include <memory>
#include <optional>

#include "variable.h"

namespace interp {

class Scope {
    std::map<std::string, std::shared_ptr<Variable>> vars;

public:
    void Declare(const std::shared_ptr<Variable>& newvar);
    std::optional<std::shared_ptr<Variable>> Lookup(const std::string& name) const;
};

class ScopeStack {
    Scope scope;
    std::shared_ptr<ScopeStack> maybeParent;

public:
    ScopeStack();
    ScopeStack(const std::shared_ptr<ScopeStack>& parent);
    void Declare(const std::shared_ptr<Variable>& newvar);
    virtual std::optional<std::shared_ptr<Variable>> Lookup(const std::string& name) const;
    virtual ~ScopeStack() = default;
};

}  // namespace interp
