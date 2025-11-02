#pragma once
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "bigint.h"
#include "syntax.h"
#include "types.h"

namespace runtime {

/*
 * If a function accepts an index, it is 1-based (the first element has index 1)
 */

class RuntimeValue : public std::enable_shared_from_this<RuntimeValue> {
public:
    virtual std::shared_ptr<runtime::Type> TypeOfValue() = 0;
    virtual std::optional<std::shared_ptr<RuntimeValue>> BinaryPlus(const RuntimeValue& other) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> BinaryMinus(const RuntimeValue& other) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> BinaryMul(const RuntimeValue& other) const;
    // Beware of integer zero division! It returns {}.
    virtual std::optional<std::shared_ptr<RuntimeValue>> BinaryDiv(const RuntimeValue& other) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> BinaryAnd(const RuntimeValue& other) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> BinaryOr(const RuntimeValue& other) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> BinaryXor(const RuntimeValue& other) const;
    virtual std::optional<std::partial_ordering> BinaryComparison(const RuntimeValue& other) const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> UnaryMinus() const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> UnaryPlus() const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> UnaryNot() const;
    virtual std::optional<std::shared_ptr<RuntimeValue>> Field(const std::string& name) const;        // a.fieldname
    virtual std::optional<std::shared_ptr<RuntimeValue>> Field(const RuntimeValue& index) const;      // a.(2 + 3)
    virtual std::optional<std::shared_ptr<RuntimeValue>> Subscript(const RuntimeValue& other) const;  // a[3 + 2]
    virtual ~RuntimeValue() = default;
};

/*
 * Has fields:
 * + Round: int = this
 * + Floor: int = this
 * + Ceil: int = this
 * + Frac: real = 0.0
 */
class IntegerValue : public RuntimeValue {
    BigInt value;

public:
    const BigInt& Value() const;
    IntegerValue(const BigInt& val);
    std::shared_ptr<runtime::Type> TypeOfValue() override;
    std::optional<std::shared_ptr<RuntimeValue>> BinaryPlus(const RuntimeValue& other) const override;
    std::optional<std::shared_ptr<RuntimeValue>> BinaryMinus(const RuntimeValue& other) const override;
    std::optional<std::shared_ptr<RuntimeValue>> BinaryMul(const RuntimeValue& other) const override;
    std::optional<std::shared_ptr<RuntimeValue>> BinaryDiv(const RuntimeValue& other) const override;
    std::optional<std::partial_ordering> BinaryComparison(const RuntimeValue& other) const override;
    std::optional<std::shared_ptr<RuntimeValue>> UnaryMinus() const override;
    std::optional<std::shared_ptr<RuntimeValue>> UnaryPlus() const override;
    std::optional<std::shared_ptr<RuntimeValue>> Field(const std::string& name) const override;
    virtual ~IntegerValue() override = default;
};

/*
 * Has fields:
 * + Round: int
 * + Floor: int
 * + Ceil: int
 * + Frac: real
 */
class RealValue : public RuntimeValue {
    long double value;

public:
    long double Value() const;
    RealValue(long double val);
    std::shared_ptr<runtime::Type> TypeOfValue() override;
    std::optional<std::shared_ptr<RuntimeValue>> BinaryPlus(const RuntimeValue& other) const override;
    std::optional<std::shared_ptr<RuntimeValue>> BinaryMinus(const RuntimeValue& other) const override;
    std::optional<std::shared_ptr<RuntimeValue>> BinaryMul(const RuntimeValue& other) const override;
    std::optional<std::shared_ptr<RuntimeValue>> BinaryDiv(const RuntimeValue& other) const override;
    std::optional<std::partial_ordering> BinaryComparison(const RuntimeValue& other) const override;
    std::optional<std::shared_ptr<RuntimeValue>> UnaryMinus() const override;
    std::optional<std::shared_ptr<RuntimeValue>> UnaryPlus() const override;
    std::optional<std::shared_ptr<RuntimeValue>> Field(const std::string& name) const override;
    virtual ~RealValue() override = default;
};

/*
 * Has fields:
 * + Split: function (string) -> []
 * + SplitWS: function () -> []
 * + Join: function ([]) -> string
 * + Lower: string
 * + Upper: string
 * + Length: int
 * + Slice: function (int, int, int) -> string  // start, stop, step
 */
class StringValue : public RuntimeValue {
    std::string value;

public:
    StringValue(const std::string& value);
    std::shared_ptr<runtime::Type> TypeOfValue() override;
    const std::string& Value() const;
    std::vector<std::string> Split(const std::string& sep) const;
    std::vector<std::string> SplitWS() const;
    std::string Join(const std::vector<std::string>&) const;
    std::string Lower() const;
    std::string Upper() const;
    // negative index does not mean "from the end"; `stop` is exclusive (do not include `stop`), step /= 0
    // "123456789".Slice(-7, 9, 4) = "15"
    std::string Slice(const BigInt& start, const BigInt& stop, const BigInt& step) const;
    std::optional<std::shared_ptr<RuntimeValue>> BinaryPlus(const RuntimeValue& other) const override;
    std::optional<std::partial_ordering> BinaryComparison(const RuntimeValue& other) const override;
    std::optional<std::shared_ptr<RuntimeValue>> Field(const std::string& name) const override;
    std::optional<std::shared_ptr<RuntimeValue>> Subscript(const RuntimeValue& other) const override;
    virtual ~StringValue() override = default;
};

class NoneValue : public RuntimeValue {
public:
    std::shared_ptr<runtime::Type> TypeOfValue() override;
    virtual ~NoneValue() override = default;
};

class BoolValue : public RuntimeValue {
    bool value;

public:
    BoolValue(bool value);
    std::shared_ptr<runtime::Type> TypeOfValue() override;
    std::optional<std::shared_ptr<RuntimeValue>> BinaryAnd(const RuntimeValue& other) const override;
    std::optional<std::shared_ptr<RuntimeValue>> BinaryOr(const RuntimeValue& other) const override;
    std::optional<std::shared_ptr<RuntimeValue>> BinaryXor(const RuntimeValue& other) const override;
    std::optional<std::shared_ptr<RuntimeValue>> UnaryNot() const override;
    virtual ~BoolValue() override = default;
};

class ArrayValue : public RuntimeValue {
public:
    std::shared_ptr<runtime::Type> TypeOfValue() override;
    std::optional<std::shared_ptr<RuntimeValue>> BinaryPlus(const RuntimeValue& other) const override;
    std::optional<std::partial_ordering> BinaryComparison(const RuntimeValue& other) const override;
    std::optional<std::shared_ptr<RuntimeValue>> Subscript(const RuntimeValue& other) const override;
    virtual ~ArrayValue() override = default;
};

class TupleValue : public RuntimeValue {
    std::vector<std::shared_ptr<RuntimeValue>> values;
    std::map<std::string, size_t> nameIndex;

public:
    TupleValue(const std::vector<std::shared_ptr<RuntimeValue>>& values,
               const std::map<std::string, size_t>& nameIndex);
    TupleValue(const std::vector<std::pair<std::optional<std::string>, std::shared_ptr<RuntimeValue>>>& vals);
    TupleValue(const TupleValue& left, const TupleValue& right);  // If names collide, keep the one from the left side
    std::vector<std::shared_ptr<RuntimeValue>> Values() const;
    std::optional<size_t> IndexByName(const std::string& name) const;
    std::optional<std::shared_ptr<RuntimeValue>> ValueByName(const std::string& name) const;
    std::optional<std::shared_ptr<RuntimeValue>> ValueByIndex(BigInt index) const;
    std::shared_ptr<runtime::Type> TypeOfValue() override;
    std::optional<std::shared_ptr<RuntimeValue>> BinaryPlus(const RuntimeValue& other) const override;
    std::optional<std::partial_ordering> BinaryComparison(const RuntimeValue& other) const override;
    std::optional<std::shared_ptr<RuntimeValue>> Field(const std::string& name) const override;
    std::optional<std::shared_ptr<RuntimeValue>> Field(const RuntimeValue& index) const override;
    virtual ~TupleValue() override = default;
};

class FuncValue : public RuntimeValue {
public:
    virtual std::variant<std::shared_ptr<RuntimeValue>, std::string> Call(
        const std::vector<std::shared_ptr<RuntimeValue>>& args) = 0;
    virtual ~FuncValue() override = default;
};

class StringSplitFunction : public FuncValue {
    std::shared_ptr<StringValue> _this;

public:
    StringSplitFunction(const std::shared_ptr<StringValue>& _this);
    std::variant<std::shared_ptr<RuntimeValue>, std::string> Call(
        const std::vector<std::shared_ptr<RuntimeValue>>& args) override;
    std::shared_ptr<runtime::Type> TypeOfValue() override;
    virtual ~StringSplitFunction() override = default;
};

class StringSplitWSFunction : public FuncValue {
    std::shared_ptr<StringValue> _this;

public:
    StringSplitWSFunction(const std::shared_ptr<StringValue>& _this);
    std::variant<std::shared_ptr<RuntimeValue>, std::string> Call(
        const std::vector<std::shared_ptr<RuntimeValue>>& args) override;
    std::shared_ptr<runtime::Type> TypeOfValue() override;
    virtual ~StringSplitWSFunction() override = default;
};

class StringJoinFunction : public FuncValue {
    std::shared_ptr<StringValue> _this;

public:
    StringJoinFunction(const std::shared_ptr<StringValue>& _this);
    std::variant<std::shared_ptr<RuntimeValue>, std::string> Call(
        const std::vector<std::shared_ptr<RuntimeValue>>& args) override;
    std::shared_ptr<runtime::Type> TypeOfValue() override;
    virtual ~StringJoinFunction() override = default;
};

class StringSliceFunction : public FuncValue {
    std::shared_ptr<StringValue> _this;

public:
    StringSliceFunction(const std::shared_ptr<StringValue>& _this);
    std::variant<std::shared_ptr<RuntimeValue>, std::string> Call(
        const std::vector<std::shared_ptr<RuntimeValue>>& args) override;
    std::shared_ptr<runtime::Type> TypeOfValue() override;
    virtual ~StringSliceFunction() override = default;
};

// No value must have UnknownType, that type is reserved for typechecking

}  // namespace runtime
