#include "runtime/values.h"

#include <compare>
#include <memory>
#include <cmath>
using namespace std;

namespace runtime {

/*
 * If a function accepts an index, it is 1-based (the first element has index 1)
 */

optional<shared_ptr<RuntimeValue>> RuntimeValue::BinaryPlus(const RuntimeValue& other) const { return {}; }
optional<shared_ptr<RuntimeValue>> RuntimeValue::BinaryMinus(const RuntimeValue& other) const { return {}; }
optional<shared_ptr<RuntimeValue>> RuntimeValue::BinaryMul(const RuntimeValue& other) const { return {}; }
optional<shared_ptr<RuntimeValue>> RuntimeValue::BinaryDiv(const RuntimeValue& other) const { return {}; }
optional<shared_ptr<RuntimeValue>> RuntimeValue::BinaryAnd(const RuntimeValue& other) const { return {}; }
optional<shared_ptr<RuntimeValue>> RuntimeValue::BinaryOr(const RuntimeValue& other) const { return {}; }
optional<shared_ptr<RuntimeValue>> RuntimeValue::BinaryXor(const RuntimeValue& other) const { return {}; }
optional<partial_ordering> RuntimeValue::BinaryComparison(const RuntimeValue& other) const { return {}; }
optional<shared_ptr<RuntimeValue>> RuntimeValue::UnaryMinus() const { return {}; }
optional<shared_ptr<RuntimeValue>> RuntimeValue::UnaryPlus() const { return {}; }
optional<shared_ptr<RuntimeValue>> RuntimeValue::UnaryNot() const { return {}; }
optional<shared_ptr<RuntimeValue>> RuntimeValue::Field(const string& name) const { return {}; }
optional<shared_ptr<RuntimeValue>> RuntimeValue::Field(const RuntimeValue& index) const { return {}; }
optional<shared_ptr<RuntimeValue>> RuntimeValue::Subscript(const RuntimeValue& other) const { return {}; }

#define NUMERIC_CLASSIFY                                              \
    const IntegerValue* aint = dynamic_cast<const IntegerValue*>(&a); \
    const IntegerValue* bint = dynamic_cast<const IntegerValue*>(&b); \
    const RealValue* areal = dynamic_cast<const RealValue*>(&a);      \
    const RealValue* breal = dynamic_cast<const RealValue*>(&b);      \
    if (!aint && !areal || !bint && !breal) return {};
#define NUMERIC_CUSTOM(operator, name, customint)                                                           \
    static optional<shared_ptr<RuntimeValue>> Numeric##name(const RuntimeValue& a, const RuntimeValue& b) { \
        NUMERIC_CLASSIFY                                                                                    \
        if (aint && bint) {                                                                                 \
            customint return make_shared<IntegerValue>(aint->Value() operator bint->Value());               \
        }                                                                                                   \
        long double lhs = aint ? aint->Value().ToFloat() : areal->Value();                                  \
        long double rhs = bint ? bint->Value().ToFloat() : breal->Value();                                  \
        return make_shared<RealValue>(lhs operator rhs);                                                    \
    }
#define NUMERIC_ARITH(operator, name) NUMERIC_CUSTOM(operator, name, )

NUMERIC_ARITH(+, Plus)
NUMERIC_ARITH(-, Minus)
NUMERIC_ARITH(*, Mul)
NUMERIC_CUSTOM(/, Div, if (!bint->Value()) return {};)

static partial_ordering OrderingFromInt(int o) {
    if (o > 0) return partial_ordering::greater;
    if (o < 0) return partial_ordering::less;
    return partial_ordering::equivalent;
}

static optional<partial_ordering> NumericComparison(const RuntimeValue& a, const RuntimeValue& b) {
    NUMERIC_CLASSIFY
    if (aint) {
        if (bint) return OrderingFromInt(aint->Value() <=> bint->Value());
        return aint->Value() <=> breal->Value();
    }
    if (bint) return areal->Value() <=> bint->Value();
    return areal->Value() <=> breal->Value();
}

IntegerValue::IntegerValue(const BigInt& val) : value(val) {}
const BigInt& IntegerValue::Value() const { return value; }
shared_ptr<runtime::Type> IntegerValue::TypeOfValue() { return make_shared<IntegerType>(); }
optional<shared_ptr<RuntimeValue>> IntegerValue::BinaryPlus(const RuntimeValue& other) const {
    return NumericPlus(*this, other);
}
optional<shared_ptr<RuntimeValue>> IntegerValue::BinaryMinus(const RuntimeValue& other) const {
    return NumericMinus(*this, other);
}
optional<shared_ptr<RuntimeValue>> IntegerValue::BinaryMul(const RuntimeValue& other) const {
    return NumericMul(*this, other);
}
optional<shared_ptr<RuntimeValue>> IntegerValue::BinaryDiv(const RuntimeValue& other) const {
    return NumericDiv(*this, other);
}
optional<partial_ordering> IntegerValue::BinaryComparison(const RuntimeValue& other) const {
    return NumericComparison(*this, other);
}
optional<shared_ptr<RuntimeValue>> IntegerValue::UnaryMinus() const {
    return make_shared<IntegerValue>(-value);
}
optional<shared_ptr<RuntimeValue>> IntegerValue::UnaryPlus() const { return make_shared<IntegerValue>(value); }
optional<shared_ptr<RuntimeValue>> IntegerValue::Field(const string& name) const {
    if (name == "Round" || name == "Floor" || name == "Ceil") return make_shared<IntegerValue>(value);
    if (name == "Frac") return make_shared<RealValue>(0);
    return {};
}

RealValue::RealValue(long double val) : value(val) {}
long double RealValue::Value() const {
    return value;
}
std::shared_ptr<runtime::Type> RealValue::TypeOfValue() {
    return make_shared<RealType>();
}
std::optional<std::shared_ptr<RuntimeValue>> RealValue::BinaryPlus(const RuntimeValue& other) const {
    return NumericPlus(*this, other);
}
std::optional<std::shared_ptr<RuntimeValue>> RealValue::BinaryMinus(const RuntimeValue& other) const {
    return NumericMinus(*this, other);
}
std::optional<std::shared_ptr<RuntimeValue>> RealValue::BinaryMul(const RuntimeValue& other) const {
    return NumericMul(*this, other);
}
std::optional<std::shared_ptr<RuntimeValue>> RealValue::BinaryDiv(const RuntimeValue& other) const {
    return NumericDiv(*this, other);
}
std::optional<std::partial_ordering> RealValue::BinaryComparison(const RuntimeValue& other) const {
    return NumericComparison(*this, other);
}
std::optional<std::shared_ptr<RuntimeValue>> RealValue::UnaryMinus() const {
    return make_shared<RealValue>(-value);
}
std::optional<std::shared_ptr<RuntimeValue>> RealValue::UnaryPlus() const {
    return make_shared<RealValue>(value);
}
std::optional<std::shared_ptr<RuntimeValue>> RealValue::Field(const std::string& name) const {
    if (name == "Round") return make_shared<IntegerValue>(BigInt(round(value)));
    if (name == "Floor") return make_shared<IntegerValue>(BigInt(floor(value)));
    if (name == "Ceil") return make_shared<IntegerValue>(BigInt(ceil(value)));
    if (name == "Frac") {
        long double res;
        if (isnan(value) || isinf(value)) res = 0;
        else if (value < 0) res = value - ceil(value);
        else res = value - floor(value);
        return make_shared<RealValue>(value);
    }
    return {};
}

// Implement class StringValue
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

// Implement class NoneValue

// Implement class BoolValue

// Implement class ArrayValue

// Implement class TupleValue

// Implement class FuncValue

// Implement class StringSplitFunction

// Implement class StringSplitWSFunction

// Implement class StringJoinFunction

// Implement class StringSliceFunction

}  // namespace runtime
