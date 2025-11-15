#pragma once
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "bigint.h"
#include "derror.h"
#include "syntax.h"
#include "types.h"

namespace runtime {

/*
 * If a function accepts an index, it is 1-based (the first element has index 1)
 */

class RuntimeValue;

using RuntimeValueResult = std::optional<std::variant<std::shared_ptr<RuntimeValue>, DRuntimeError>>;

class RuntimeValue : public std::enable_shared_from_this<RuntimeValue> {
public:
    virtual void DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>& recGuard) const = 0;
    void PrintSelf(std::ostream& out);
    virtual std::shared_ptr<runtime::Type> TypeOfValue() const = 0;
    virtual RuntimeValueResult BinaryPlus(const RuntimeValue& other) const;
    virtual RuntimeValueResult BinaryMinus(const RuntimeValue& other) const;
    virtual RuntimeValueResult BinaryMul(const RuntimeValue& other) const;
    virtual RuntimeValueResult BinaryDiv(const RuntimeValue& other) const;
    virtual RuntimeValueResult BinaryAnd(const RuntimeValue& other) const;
    virtual RuntimeValueResult BinaryOr(const RuntimeValue& other) const;
    virtual RuntimeValueResult BinaryXor(const RuntimeValue& other) const;
    virtual std::optional<std::partial_ordering> BinaryComparison(const RuntimeValue& other) const;
    virtual RuntimeValueResult UnaryMinus() const;
    virtual RuntimeValueResult UnaryPlus() const;
    virtual RuntimeValueResult UnaryNot() const;
    virtual RuntimeValueResult Field(const std::string& name) const;        // a.fieldname
    virtual RuntimeValueResult Field(const RuntimeValue& index) const;      // a.(2 + 3)
    virtual RuntimeValueResult Subscript(const RuntimeValue& other) const;  // a[3 + 2]
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
    void DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>& recGuard) const override;
    const BigInt& Value() const;
    IntegerValue(const BigInt& val);
    std::shared_ptr<runtime::Type> TypeOfValue() const override;
    RuntimeValueResult BinaryPlus(const RuntimeValue& other) const override;
    RuntimeValueResult BinaryMinus(const RuntimeValue& other) const override;
    RuntimeValueResult BinaryMul(const RuntimeValue& other) const override;
    RuntimeValueResult BinaryDiv(const RuntimeValue& other) const override;
    std::optional<std::partial_ordering> BinaryComparison(const RuntimeValue& other) const override;
    RuntimeValueResult UnaryMinus() const override;
    RuntimeValueResult UnaryPlus() const override;
    RuntimeValueResult Field(const std::string& name) const override;
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
    void DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>& recGuard) const override;
    long double Value() const;
    RealValue(long double val);
    std::shared_ptr<runtime::Type> TypeOfValue() const override;
    RuntimeValueResult BinaryPlus(const RuntimeValue& other) const override;
    RuntimeValueResult BinaryMinus(const RuntimeValue& other) const override;
    RuntimeValueResult BinaryMul(const RuntimeValue& other) const override;
    RuntimeValueResult BinaryDiv(const RuntimeValue& other) const override;
    std::optional<std::partial_ordering> BinaryComparison(const RuntimeValue& other) const override;
    RuntimeValueResult UnaryMinus() const override;
    RuntimeValueResult UnaryPlus() const override;
    RuntimeValueResult Field(const std::string& name) const override;
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
    void DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>& recGuard) const override;
    StringValue(const std::string& value);
    std::shared_ptr<runtime::Type> TypeOfValue() const override;
    const std::string& Value() const;
    std::vector<std::string> Split(const std::string& sep) const;
    std::vector<std::string> SplitWS() const;
    std::string Join(const std::vector<std::string>&) const;
    std::string Lower() const;
    std::string Upper() const;
    // negative index does not mean "from the end"; `stop` is exclusive (do not include `stop`), step /= 0
    // "123456789".Slice(-7, 9, 4) = "15"
    std::string Slice(const BigInt& start, const BigInt& stop, const BigInt& step) const;
    RuntimeValueResult BinaryPlus(const RuntimeValue& other) const override;
    std::optional<std::partial_ordering> BinaryComparison(const RuntimeValue& other) const override;
    RuntimeValueResult Field(const std::string& name) const override;
    RuntimeValueResult Subscript(const RuntimeValue& other) const override;
    virtual ~StringValue() override = default;
};

class NoneValue : public RuntimeValue {
public:
    void DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>& recGuard) const override;
    std::shared_ptr<runtime::Type> TypeOfValue() const override;
    virtual ~NoneValue() override = default;
};

class BoolValue : public RuntimeValue {
    bool value;

public:
    void DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>& recGuard) const override;
    BoolValue(bool value);
    bool Value() const;
    std::shared_ptr<runtime::Type> TypeOfValue() const override;
    RuntimeValueResult BinaryAnd(const RuntimeValue& other) const override;
    RuntimeValueResult BinaryOr(const RuntimeValue& other) const override;
    RuntimeValueResult BinaryXor(const RuntimeValue& other) const override;
    RuntimeValueResult UnaryNot() const override;
    virtual ~BoolValue() override = default;
};

class ArrayValue : public RuntimeValue {
public:
    void DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>& recGuard) const override;
    std::map<BigInt, std::shared_ptr<RuntimeValue>> Value;
    ArrayValue(const std::vector<std::shared_ptr<RuntimeValue>>& arr);
    ArrayValue(const std::map<BigInt, std::shared_ptr<RuntimeValue>>& mp);
    std::shared_ptr<runtime::Type> TypeOfValue() const override;
    RuntimeValueResult BinaryPlus(const RuntimeValue& other) const override;
    std::optional<std::partial_ordering> BinaryComparison(const RuntimeValue& other) const override;
    RuntimeValueResult Subscript(const RuntimeValue& other) const override;
    void AssignItem(const BigInt& index, const std::shared_ptr<RuntimeValue>& other);
    virtual ~ArrayValue() override = default;
};

class TupleValue : public RuntimeValue {
    std::vector<std::shared_ptr<RuntimeValue>> values;
    std::map<std::string, size_t> nameIndex;

public:
    void DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>& recGuard) const override;
    TupleValue(const std::vector<std::shared_ptr<RuntimeValue>>& values,
               const std::map<std::string, size_t>& nameIndex);
    TupleValue(const std::vector<std::pair<std::optional<std::string>, std::shared_ptr<RuntimeValue>>>& vals);
    TupleValue(const TupleValue& left, const TupleValue& right);  // If names collide, keep the one from the left side
    std::vector<std::shared_ptr<RuntimeValue>> Values() const;
    std::optional<size_t> IndexByName(const std::string& name) const;  // 0-based
    RuntimeValueResult ValueByName(const std::string& name) const;
    RuntimeValueResult ValueByIndex(BigInt index) const;
    std::shared_ptr<runtime::Type> TypeOfValue() const override;
    RuntimeValueResult BinaryPlus(const RuntimeValue& other) const override;
    RuntimeValueResult Field(const std::string& name) const override;
    RuntimeValueResult Field(const RuntimeValue& index) const override;
    bool AssignNamedField(const std::string& name, const std::shared_ptr<RuntimeValue>& val);
    bool AssignIndexedField(const BigInt& index, const std::shared_ptr<RuntimeValue>& val);
    virtual ~TupleValue() override = default;
};

class FuncValue : public RuntimeValue {
public:
    virtual RuntimeValueResult Call(const std::vector<std::shared_ptr<RuntimeValue>>& args) const = 0;
    virtual ~FuncValue() override = default;
};

class StringSplitFunction : public FuncValue {
    std::shared_ptr<const StringValue> _this;

public:
    void DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>& recGuard) const override;
    StringSplitFunction(const std::shared_ptr<const StringValue>& _this);
    RuntimeValueResult Call(const std::vector<std::shared_ptr<RuntimeValue>>& args) const override;
    std::shared_ptr<runtime::Type> TypeOfValue() const override;
    virtual ~StringSplitFunction() override = default;
};

class StringSplitWSFunction : public FuncValue {
    std::shared_ptr<const StringValue> _this;

public:
    void DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>& recGuard) const override;
    StringSplitWSFunction(const std::shared_ptr<const StringValue>& _this);
    RuntimeValueResult Call(const std::vector<std::shared_ptr<RuntimeValue>>& args) const override;
    std::shared_ptr<runtime::Type> TypeOfValue() const override;
    virtual ~StringSplitWSFunction() override = default;
};

class StringJoinFunction : public FuncValue {
    std::shared_ptr<const StringValue> _this;

public:
    void DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>& recGuard) const override;
    StringJoinFunction(const std::shared_ptr<const StringValue>& _this);
    RuntimeValueResult Call(const std::vector<std::shared_ptr<RuntimeValue>>& args) const override;
    std::shared_ptr<runtime::Type> TypeOfValue() const override;
    virtual ~StringJoinFunction() override = default;
};

class StringSliceFunction : public FuncValue {
    std::shared_ptr<const StringValue> _this;

public:
    void DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>& recGuard) const override;
    StringSliceFunction(const std::shared_ptr<const StringValue>& _this);
    RuntimeValueResult Call(const std::vector<std::shared_ptr<RuntimeValue>>& args) const override;
    std::shared_ptr<runtime::Type> TypeOfValue() const override;
    virtual ~StringSliceFunction() override = default;
};

// No value must have UnknownType, that type is reserved for typechecking

}  // namespace runtime
