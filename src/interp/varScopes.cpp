#include "interp/varScopes.h"

#include <stdexcept>
using namespace std;

namespace interp {

void Scope::Declare(const shared_ptr<Variable>& newvar) {
    auto result = vars.try_emplace(newvar->Name(), newvar);
    if (!result.second)
        throw runtime_error("Tried to declare a new variable with a taken name \"" + newvar->Name() + "\"");
}

optional<shared_ptr<Variable>> Scope::Lookup(const string& name) const {
    auto iter = vars.find(name);
    if (iter == vars.end()) return {};
    return {iter->second};
}

ScopeStack::ScopeStack() = default;

ScopeStack::ScopeStack(const shared_ptr<ScopeStack>& parent) : maybeParent(parent) {}

void ScopeStack::Declare(const shared_ptr<Variable>& newvar) { scope.Declare(newvar); }

optional<shared_ptr<Variable>> ScopeStack::Lookup(const string& name) const {
    auto res = scope.Lookup(name);
    if (!res && maybeParent) return maybeParent->Lookup(name);
    return res;
}

}  // namespace interp
