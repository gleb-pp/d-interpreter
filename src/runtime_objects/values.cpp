#include "runtime/values.h"

#include <algorithm>
#include <compare>
#include <memory>
#include <cmath>
#include "runtime/types.h"
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
shared_ptr<runtime::Type> RealValue::TypeOfValue() {
    return make_shared<RealType>();
}
optional<shared_ptr<RuntimeValue>> RealValue::BinaryPlus(const RuntimeValue& other) const {
    return NumericPlus(*this, other);
}
optional<shared_ptr<RuntimeValue>> RealValue::BinaryMinus(const RuntimeValue& other) const {
    return NumericMinus(*this, other);
}
optional<shared_ptr<RuntimeValue>> RealValue::BinaryMul(const RuntimeValue& other) const {
    return NumericMul(*this, other);
}
optional<shared_ptr<RuntimeValue>> RealValue::BinaryDiv(const RuntimeValue& other) const {
    return NumericDiv(*this, other);
}
optional<partial_ordering> RealValue::BinaryComparison(const RuntimeValue& other) const {
    return NumericComparison(*this, other);
}
optional<shared_ptr<RuntimeValue>> RealValue::UnaryMinus() const {
    return make_shared<RealValue>(-value);
}
optional<shared_ptr<RuntimeValue>> RealValue::UnaryPlus() const {
    return make_shared<RealValue>(value);
}
optional<shared_ptr<RuntimeValue>> RealValue::Field(const string& name) const {
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

StringValue::StringValue(const string& value) : value(value) {}
shared_ptr<runtime::Type> StringValue::TypeOfValue() {
    return make_shared<StringType>();
}
const string& StringValue::Value() const {
    return value;
}
vector<string> StringValue::Split(const string& sep) const {
    if (sep.empty()) {
        vector<string> res;
        std::ranges::transform(value, std::back_inserter(res), [](char ch) { return string(1, ch); });
        return res;
    }
    long nsep = sep.size();
    vector<long> zsep(nsep);
    long l = 0, r = 0;
    for (long i = 1; i < nsep; i++) {
        long& res = zsep[i];
        if (i < r) res = max(r - i, zsep[i - l]);
        while (i + res < nsep && sep[i + res] == sep[res]) ++res;
        if (i + res > r) {
            l = i;
            r = i + res;
        }
    }
    l = 0; r = 0;
    long n = value.size();
    vector<long> z(n);
    for (long i = 0; i < n; i++) {
        long& res = z[i];
        if (i > r) res = max(r - i, z[i - l]);
        while (i + res < n && res < nsep && value[i + res] == sep[res]) ++res;
        if (i + res > r) {
            r = i + res;
            l = i;
        }
    }
    vector<string> res;
    long pos = 0;
    for (long i = 0; i < n; i++) if (z[i] == nsep) {
        res.push_back(value.substr(pos, i - pos));
        i += nsep - 1;
        pos = i;
    }
    res.push_back(value.substr(pos));
    return res;
}
vector<string> StringValue::SplitWS() const {
    long n = value.size();
    long pos = 0;
    vector<string> res;
    while (pos < n && isspace(value[pos])) ++pos;
    while (pos < n) {
        long i = pos + 1;
        while (i < n && !isspace(value[i])) ++i;
        res.push_back(value.substr(pos, i - pos));
        while (i < n && isspace(value[i])) ++i;
        pos = i;
    }
    return res;
}
string StringValue::Join(const vector<string>& v) const {
    if (v.empty()) return "";
    string res;
    size_t anssize = (v.size() - 1) * value.size();
    for (auto& i : v) anssize += i.size();
    res.reserve(anssize);
    bool first = true;
    for (auto& i : v) {
        if (!first) res += value;
        first = false;
        res += i;
    }
    return res;
}
string StringValue::Lower() const {
    string res = value;
    std::ranges::transform(res, res.begin(), [](char ch) { return tolower(ch); });
    return res;
}
string StringValue::Upper() const {
    string res = value;
    std::ranges::transform(res, res.begin(), [](char ch) { return toupper(ch); });
    return res;
}
string StringValue::Slice(const BigInt& start, const BigInt& stop, const BigInt& step) const {
    if (!step) return "";
    long n = value.size();
    if (step.IsNegative()) {
        auto absstep = -step;
        auto fromend = (BigInt(n) - start) % step;  // 0-based
        
    }
}
optional<shared_ptr<RuntimeValue>> StringValue::BinaryPlus(const RuntimeValue& other) const {

}
optional<partial_ordering> StringValue::BinaryComparison(const RuntimeValue& other) const {

}
optional<shared_ptr<RuntimeValue>> StringValue::Field(const string& name) const {

}
optional<shared_ptr<RuntimeValue>> StringValue::Subscript(const RuntimeValue& other) const {

}

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
