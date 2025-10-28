#pragma once
#include <map>
#include <vector>
#include <string>
#include "types.h"
#include "bigint.h"
#include "syntax.h"

namespace runtime {

class RuntimeValue {
public:
    virtual std::shared_ptr<runtime::Type> TypeOfValue() = 0;
    virtual std::optional<std::shared_ptr<RuntimeValue>> BinaryPlus(const RuntimeValue& other) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> BinaryMinus(const RuntimeValue& other) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> BinaryMul(const RuntimeValue& other) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> BinaryDiv(const RuntimeValue& other) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> BinaryAnd(const RuntimeValue& other) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> BinaryOr(const RuntimeValue& other) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> BinaryXor(const RuntimeValue& other) const;
    virtual std::optional<int> BinaryComparison(const RuntimeValue& other) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> UnaryMinus(const RuntimeValue& other) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> UnaryPlus(const RuntimeValue& other) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> UnaryNot(const RuntimeValue& other) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> Field(const std::string& name) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> Field(const RuntimeValue& index) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> Subscript(const RuntimeValue& other) const;
    virtual ~RuntimeValue() = default;
};

class Integer : public RuntimeValue {

};

}
