#include "semantic/valueTimeline.h"

#include "locators/locator.h"
#include "runtime.h"
#include "runtime/types.h"
#include "runtime/values.h"
using namespace std;

namespace semantic {

ValueTimeline::Var::Var(const locators::SpanLocator& declloc)
    : val(make_shared<runtime::NoneValue>()), declaration(declloc) {}

optional<pair<size_t, const ValueTimeline::Var*>> ValueTimeline::Lookup(const std::string& name) const {
    for (long i = static_cast<long>(stack.size()) - 1; i >= 0l; --i) {
        auto& scope = stack[i];
        auto iter = scope.vars.find(name);
        if (iter == scope.vars.end()) continue;
        return make_pair(i, &iter->second);
    }
    return {};
}

optional<pair<size_t, ValueTimeline::Var*>> ValueTimeline::Lookup(const std::string& name) {
    for (long i = static_cast<long>(stack.size()) - 1; i >= 0l; --i) {
        auto& scope = stack[i];
        auto iter = scope.vars.find(name);
        if (iter == scope.vars.end()) continue;
        return make_pair(i, &iter->second);
    }
    return {};
}

optional<runtime::TypeOrValue> ValueTimeline::LookupVariable(const string& name) {
    auto v = Lookup(name);
    if (!v) return {};
    auto& p = *v;
    if (blindScopeIndices.size() && p.first < blindScopeIndices.back()) return make_shared<runtime::UnknownType>();
    p.second->used = true;
    p.second->lastUnusedAssignments.clear();
    return p.second->val;
}

void ValueTimeline::MakeAllUnknown() {
    auto unk = make_shared<runtime::UnknownType>();
    for (auto& scope : stack)
        for (auto& kv : scope.vars) {
            auto& var = kv.second;
            var.val = unk;
            var.used = true;
            var.lastUnusedAssignments.clear();
        }
}

void ValueTimeline::StartScope() { stack.emplace_back(); }

void ValueTimeline::StartBlindScope() {
    blindScopeIndices.push_back(stack.size());
    StartScope();
}

ScopeStats ValueTimeline::EndScope() {
    ScopeStats res;
    auto top = std::move(stack.back());
    stack.pop_back();

    res.referencedExternals = top.externalReferences;
    for (auto& kv : top.vars) {
        auto& var = kv.second;
        if (!var.used)
            res.variablesNeverUsed.emplace_back(kv.first, var.declaration);
        else
            for (auto& asg : var.lastUnusedAssignments) {
                res.uselessAssignments.emplace_back(kv.first, *asg);
            }
    }
    if (stack.size()) {
        auto& newtop = stack.back();
        for (const auto& kv : top.externalReferences)
            if (!newtop.vars.contains(kv.first)) {
                bool& wasAssigned = newtop.externalReferences[kv.first];
                wasAssigned = wasAssigned || kv.second;
            }
    }
    return res;
}

bool ValueTimeline::AssignType(const string& name, const shared_ptr<runtime::Type>& type, locators::SpanLocator pos) {
    auto search = Lookup(name);
    if (!search) return false;
    auto& [scopeindex, var] = *search;
    auto& topscope = stack.back();
    if (scopeindex != stack.size() - 1) topscope.externalReferences[name] = true;
    var->val = type;
    var->lastUnusedAssignments = {make_shared<locators::SpanLocator>(pos)};
    var->used = true;
    return true;
}

bool ValueTimeline::AssignValue(const string& name, const shared_ptr<runtime::RuntimeValue>& precomputed,
                                locators::SpanLocator pos) {
    auto search = Lookup(name);
    if (!search) return false;
    auto& [scopeindex, var] = *search;
    auto& topscope = stack.back();
    if (scopeindex != stack.size() - 1) topscope.externalReferences[name] = true;
    var->val = precomputed;
    var->lastUnusedAssignments = {make_shared<locators::SpanLocator>(pos)};
    var->used = true;
    return true;
}

bool ValueTimeline::Assign(const string& name, const runtime::TypeOrValue& precomputed, locators::SpanLocator pos) {
    if (precomputed.index()) return AssignValue(name, get<1>(precomputed), pos);
    return AssignType(name, get<0>(precomputed), pos);
}

bool ValueTimeline::AssignUnknownButUsed(const string& name) {
    auto search = Lookup(name);
    if (!search) return false;
    auto& [scopeindex, var] = *search;
    auto& topscope = stack.back();
    if (scopeindex != stack.size() - 1) topscope.externalReferences[name] = true;
    var->val = make_shared<runtime::UnknownType>();
    var->lastUnusedAssignments.clear();
    var->used = true;
    return true;
}

bool ValueTimeline::Declare(const string& name, locators::SpanLocator pos) {
    auto& topscope = stack.back();
    auto insertres = topscope.vars.emplace(name, Var(pos));
    return insertres.second;
}

static void GeneralizeValue(runtime::TypeOrValue& dest, const runtime::TypeOrValue& src) {
    if (dest.index() && src.index() && get<1>(dest) == get<1>(src)) return;
    auto lefttype = dest.index() ? get<1>(dest)->TypeOfValue() : get<0>(dest);
    auto righttype = src.index() ? get<1>(src)->TypeOfValue() : get<0>(src);
    dest = lefttype->Generalize(*righttype);
}

// `used` is OR'ed
// useless assignments are concatenated
// vals are generalized
// externalReferences are OR'ed
void ValueTimeline::MergeTimelines(const ValueTimeline& other) {
    size_t n = stack.size();
#ifdef DINTERP_DEBUG
    if (other.stack.size() != n) throw std::runtime_error("stack sizes were different when merging timelines");
    if (blindScopeIndices != other.blindScopeIndices)
        throw std::runtime_error("blindScopeIndices were different when merging timelines");
#endif
    for (size_t i = 0; i < n; i++) {
        auto& myscope = stack[i];
        const auto& srcscope = other.stack[i];
        for (auto& kv : srcscope.externalReferences) {
            bool& asg = myscope.externalReferences[kv.first];
            asg = asg || kv.second;
        }
#ifdef DINTERP_DEBUG
        if (myscope.vars.size() != srcscope.vars.size())
            throw std::runtime_error("variable counts were different when merging timelines");
#endif
        auto destiter = myscope.vars.begin();
        auto destend = myscope.vars.end();
        auto srciter = srcscope.vars.begin();
        for (; destiter != destend; ++destiter, ++srciter) {
#ifdef DINTERP_DEBUG
            if (destiter->first != srciter->first)
                throw std::runtime_error("variable names were different when merging timelines");
#endif
            auto& destvar = destiter->second;
            const auto& srcvar = srciter->second;
            destvar.used = destvar.used || srcvar.used;
            auto& destunused = destvar.lastUnusedAssignments;
            auto& srcunused = srcvar.lastUnusedAssignments;
            destunused.insert(srcunused.begin(), srcunused.end());
            GeneralizeValue(destvar.val, srcvar.val);
        }
    }
}

}  // namespace semantic
