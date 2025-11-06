#include "runtime/values.h"

#include <algorithm>
#include <cmath>
#include <compare>
#include <iterator>
#include <memory>
#include <sstream>

#include "runtime/types.h"
using namespace std;

namespace runtime {

/*
 * If a function accepts an index, it is 1-based (the first element has index 1)
 */

void RuntimeValue::PrintSelf(ostream& out) {
    set<shared_ptr<const RuntimeValue>> guard;
    DoPrintSelf(out, guard);
}
RuntimeValueResult RuntimeValue::BinaryPlus([[maybe_unused]] const RuntimeValue& other) const { return {}; }
RuntimeValueResult RuntimeValue::BinaryMinus([[maybe_unused]] const RuntimeValue& other) const { return {}; }
RuntimeValueResult RuntimeValue::BinaryMul([[maybe_unused]] const RuntimeValue& other) const { return {}; }
RuntimeValueResult RuntimeValue::BinaryDiv([[maybe_unused]] const RuntimeValue& other) const { return {}; }
RuntimeValueResult RuntimeValue::BinaryAnd([[maybe_unused]] const RuntimeValue& other) const { return {}; }
RuntimeValueResult RuntimeValue::BinaryOr([[maybe_unused]] const RuntimeValue& other) const { return {}; }
RuntimeValueResult RuntimeValue::BinaryXor([[maybe_unused]] const RuntimeValue& other) const { return {}; }
optional<partial_ordering> RuntimeValue::BinaryComparison([[maybe_unused]] const RuntimeValue& other) const {
    return {};
}
RuntimeValueResult RuntimeValue::UnaryMinus() const { return {}; }
RuntimeValueResult RuntimeValue::UnaryPlus() const { return {}; }
RuntimeValueResult RuntimeValue::UnaryNot() const { return {}; }
RuntimeValueResult RuntimeValue::Field([[maybe_unused]] const string& name) const { return {}; }
RuntimeValueResult RuntimeValue::Field([[maybe_unused]] const RuntimeValue& index) const { return {}; }
RuntimeValueResult RuntimeValue::Subscript([[maybe_unused]] const RuntimeValue& other) const { return {}; }

#define NUMERIC_CLASSIFY                                              \
    const IntegerValue* aint = dynamic_cast<const IntegerValue*>(&a); \
    const IntegerValue* bint = dynamic_cast<const IntegerValue*>(&b); \
    const RealValue* areal = dynamic_cast<const RealValue*>(&a);      \
    const RealValue* breal = dynamic_cast<const RealValue*>(&b);      \
    if ((!aint && !areal) || (!bint && !breal)) return {};
#define NUMERIC_CUSTOM(operator, name, customint)                                             \
    static RuntimeValueResult Numeric##name(const RuntimeValue& a, const RuntimeValue& b) {   \
        NUMERIC_CLASSIFY                                                                      \
        if (aint && bint) {                                                                   \
            customint return make_shared<IntegerValue>(aint->Value() operator bint->Value()); \
        }                                                                                     \
        long double lhs = aint ? aint->Value().ToFloat() : areal->Value();                    \
        long double rhs = bint ? bint->Value().ToFloat() : breal->Value();                    \
        return make_shared<RealValue>(lhs operator rhs);                                      \
    }
#define NUMERIC_ARITH(operator, name) NUMERIC_CUSTOM(operator, name, )

NUMERIC_ARITH(+, Plus)
NUMERIC_ARITH(-, Minus)
NUMERIC_ARITH(*, Mul)
NUMERIC_CUSTOM(/, Div, if (!bint->Value()) return runtime_error("Integer division by 0");)

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
shared_ptr<runtime::Type> IntegerValue::TypeOfValue() const { return make_shared<IntegerType>(); }
RuntimeValueResult IntegerValue::BinaryPlus(const RuntimeValue& other) const { return NumericPlus(*this, other); }
RuntimeValueResult IntegerValue::BinaryMinus(const RuntimeValue& other) const { return NumericMinus(*this, other); }
RuntimeValueResult IntegerValue::BinaryMul(const RuntimeValue& other) const { return NumericMul(*this, other); }
RuntimeValueResult IntegerValue::BinaryDiv(const RuntimeValue& other) const { return NumericDiv(*this, other); }
optional<partial_ordering> IntegerValue::BinaryComparison(const RuntimeValue& other) const {
    return NumericComparison(*this, other);
}
RuntimeValueResult IntegerValue::UnaryMinus() const { return make_shared<IntegerValue>(-value); }
RuntimeValueResult IntegerValue::UnaryPlus() const { return make_shared<IntegerValue>(value); }
RuntimeValueResult IntegerValue::Field(const string& name) const {
    if (name == "Round" || name == "Floor" || name == "Ceil") return make_shared<IntegerValue>(value);
    if (name == "Frac") return make_shared<RealValue>(0);
    return {};
}
void IntegerValue::DoPrintSelf(ostream& out, set<shared_ptr<const RuntimeValue>>&) const { out << value.ToString(); }

RealValue::RealValue(long double val) : value(val) {}
long double RealValue::Value() const { return value; }
shared_ptr<runtime::Type> RealValue::TypeOfValue() const { return make_shared<RealType>(); }
RuntimeValueResult RealValue::BinaryPlus(const RuntimeValue& other) const { return NumericPlus(*this, other); }
RuntimeValueResult RealValue::BinaryMinus(const RuntimeValue& other) const { return NumericMinus(*this, other); }
RuntimeValueResult RealValue::BinaryMul(const RuntimeValue& other) const { return NumericMul(*this, other); }
RuntimeValueResult RealValue::BinaryDiv(const RuntimeValue& other) const { return NumericDiv(*this, other); }
optional<partial_ordering> RealValue::BinaryComparison(const RuntimeValue& other) const {
    return NumericComparison(*this, other);
}
RuntimeValueResult RealValue::UnaryMinus() const { return make_shared<RealValue>(-value); }
RuntimeValueResult RealValue::UnaryPlus() const { return make_shared<RealValue>(value); }
RuntimeValueResult RealValue::Field(const string& name) const {
    if (name == "Round") return make_shared<IntegerValue>(BigInt(round(value)));
    if (name == "Floor") return make_shared<IntegerValue>(BigInt(floor(value)));
    if (name == "Ceil") return make_shared<IntegerValue>(BigInt(ceil(value)));
    if (name == "Frac") {
        long double res;
        if (isnan(value) || isinf(value))
            res = 0;
        else if (value < 0)
            res = value - ceil(value);
        else
            res = value - floor(value);
        return make_shared<RealValue>(res);
    }
    return {};
}
void RealValue::DoPrintSelf(ostream& out, set<shared_ptr<const RuntimeValue>>&) const { out << value; }

StringValue::StringValue(const string& value) : value(value) {}
shared_ptr<runtime::Type> StringValue::TypeOfValue() const { return make_shared<StringType>(); }
const string& StringValue::Value() const { return value; }
vector<string> StringValue::Split(const string& sep) const {
    if (sep.empty()) {
        vector<string> res;
        ranges::transform(value, back_inserter(res), [](char ch) { return string(1, ch); });
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
    l = 0;
    r = 0;
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
    for (long i = 0; i < n; i++)
        if (z[i] == nsep) {
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
    ranges::transform(res, res.begin(), [](char ch) { return tolower(ch); });
    return res;
}
string StringValue::Upper() const {
    string res = value;
    ranges::transform(res, res.begin(), [](char ch) { return toupper(ch); });
    return res;
}
string StringValue::Slice(const BigInt& start, const BigInt& stop, const BigInt& step) const {
    if (!step) return "";
    long n = value.size();
    if (step.IsNegative()) {
        if (start <= 0 || stop >= start) return "";
        auto absstep = -step;
        BigInt fromend = BigInt(n) - start;  // 0-based
        if (fromend >= n) return "";
        if (fromend < 0) fromend.DivLeaveMod(absstep);
        long lstart = n - start - 1;                   // 0-based
        long lstop = max(0l, stop.ClampToLong()) - 1;  // 0-based
        if (step > n) return string(1, value[lstart]);
        long lstep = absstep.ClampToLong();
        string res;
        for (long i = lstart; i > lstop; i -= lstep) res.push_back(value[i]);
        return res;
    }
    if (start > n || stop <= start) return "";
    auto zstart = start - BigInt(1);  // 0-based
    if (zstart < 0) {
        zstart.DivLeaveMod(step);
        if (zstart >= n) return "";
    }
    long lstart = zstart.ClampToLong();                 // 0-based
    long lend = stop > n ? n : stop.ClampToLong() - 1;  // 0-based
    if (step > n) return string(1, value[lstart]);
    long lstep = step.ClampToLong();
    string res;
    for (long i = lstart; i < lend; i += lstep) res.push_back(value[i]);
    return res;
}
RuntimeValueResult StringValue::BinaryPlus(const RuntimeValue& other) const {
    auto p = dynamic_cast<const StringValue*>(&other);
    if (!p) return {};
    return make_shared<StringValue>(value + p->value);
}
optional<partial_ordering> StringValue::BinaryComparison(const RuntimeValue& other) const {
    auto p = dynamic_cast<const StringValue*>(&other);
    if (!p) return {};
    return value <=> p->value;
}
RuntimeValueResult StringValue::Field(const string& name) const {
    if (name == "Split")
        return make_shared<StringSplitFunction>(dynamic_pointer_cast<const StringValue>(shared_from_this()));
    if (name == "SplitWS")
        return make_shared<StringSplitWSFunction>(dynamic_pointer_cast<const StringValue>(shared_from_this()));
    if (name == "Join")
        return make_shared<StringJoinFunction>(dynamic_pointer_cast<const StringValue>(shared_from_this()));
    if (name == "Lower") return make_shared<StringValue>(Lower());
    if (name == "Upper") return make_shared<StringValue>(Upper());
    if (name == "Length") return make_shared<IntegerValue>(value.size());
    if (name == "Slice")
        return make_shared<StringSliceFunction>(dynamic_pointer_cast<const StringValue>(shared_from_this()));
    return {};
}
RuntimeValueResult StringValue::Subscript(const RuntimeValue& other) const {
    auto intval = dynamic_cast<const IntegerValue*>(&other);
    if (!intval) return {};
    auto& bigint = intval->Value();
    if (bigint <= 0 || bigint > value.size()) return runtime_error("String index out of range");
    long ind = bigint.ClampToLong() - 1;
    return make_shared<StringValue>(string(1, value[ind]));
}
void StringValue::DoPrintSelf(ostream& out, set<shared_ptr<const RuntimeValue>>&) const { out << value; }

shared_ptr<Type> NoneValue::TypeOfValue() const { return make_shared<NoneType>(); }
void NoneValue::DoPrintSelf(ostream& out, set<shared_ptr<const RuntimeValue>>&) const { out << "<none>"; }

BoolValue::BoolValue(bool value) : value(value) {}
bool BoolValue::Value() const { return value; }
shared_ptr<runtime::Type> BoolValue::TypeOfValue() const { return make_shared<BoolType>(); }
RuntimeValueResult BoolValue::BinaryAnd(const RuntimeValue& other) const {
    auto p = dynamic_cast<const BoolValue*>(&other);
    if (!p) return {};
    return make_shared<BoolValue>(value && p->value);
}
RuntimeValueResult BoolValue::BinaryOr(const RuntimeValue& other) const {
    auto p = dynamic_cast<const BoolValue*>(&other);
    if (!p) return {};
    return make_shared<BoolValue>(value || p->value);
}
RuntimeValueResult BoolValue::BinaryXor(const RuntimeValue& other) const {
    auto p = dynamic_cast<const BoolValue*>(&other);
    if (!p) return {};
    return make_shared<BoolValue>(value != p->value);
}
RuntimeValueResult BoolValue::UnaryNot() const { return make_shared<BoolValue>(!value); }
void BoolValue::DoPrintSelf(ostream& out, set<shared_ptr<const RuntimeValue>>&) const {
    out << (value ? "true" : "false");
}

ArrayValue::ArrayValue(const vector<shared_ptr<RuntimeValue>>& arr) {
    size_t i = 0;
    for (auto& pvalue : arr) Value[BigInt(++i)] = pvalue;
}
ArrayValue::ArrayValue(const map<BigInt, shared_ptr<RuntimeValue>>& mp) : Value(mp) {}
shared_ptr<runtime::Type> ArrayValue::TypeOfValue() const { return make_shared<ArrayType>(); }
RuntimeValueResult ArrayValue::BinaryPlus(const RuntimeValue& other) const {
    auto p = dynamic_cast<const ArrayValue*>(&other);
    if (!p) return {};
    if (Value.empty()) return make_shared<ArrayValue>(p->Value);
    if (p->Value.empty()) return make_shared<ArrayValue>(Value);
    BigInt d = Value.rbegin()->first - p->Value.begin()->first + BigInt(1);
    shared_ptr<ArrayValue> result = make_shared<ArrayValue>(Value);
    map<BigInt, shared_ptr<RuntimeValue>>& dest = result->Value;
    for (auto& kv : p->Value) dest[kv.first + d] = kv.second;
    return result;
}
optional<partial_ordering> ArrayValue::BinaryComparison(const RuntimeValue& other) const {
    auto p = dynamic_cast<const ArrayValue*>(&other);
    if (!p) return {};
    if (Value == p->Value) return partial_ordering::equivalent;
    return partial_ordering::unordered;
}
RuntimeValueResult ArrayValue::Subscript(const RuntimeValue& other) const {
    auto intval = dynamic_cast<const IntegerValue*>(&other);
    if (!intval) return {};
    auto iter = Value.find(intval->Value());
    if (iter == Value.end()) return runtime_error("Array index not found");
    return iter->second;
}
void ArrayValue::AssignItem(const BigInt& index, const shared_ptr<RuntimeValue>& other) { Value[index] = other; }
void ArrayValue::DoPrintSelf(ostream& out, set<shared_ptr<const RuntimeValue>>& recGuard) const {
    auto ins = recGuard.insert(shared_from_this());
    if (!ins.second) {
        out << "[...]";
        return;
    }
    bool first = true;
    out << "[ ";
    for (auto& kv : this->Value) {
        if (!first) out << ", ";
        first = false;
        out << "[" << kv.first.ToString() << "] ";
        kv.second->DoPrintSelf(out, recGuard);
    }
    out << " ]";
    recGuard.erase(ins.first);
}

TupleValue::TupleValue(const vector<shared_ptr<RuntimeValue>>& values, const map<string, size_t>& nameIndex)
    : values(values), nameIndex(nameIndex) {}
TupleValue::TupleValue(const vector<pair<optional<string>, shared_ptr<RuntimeValue>>>& vals) {
    size_t n = vals.size();
    values.reserve(n);
    ranges::transform(vals, back_inserter(values),
                      [](const pair<optional<string>, shared_ptr<RuntimeValue>>& val) { return val.second; });
    for (size_t i = 0; i < n; i++)
        if (vals[i].first) nameIndex[*vals[i].first] = i;
}
TupleValue::TupleValue(const TupleValue& left, const TupleValue& right)
    : values(left.values), nameIndex(left.nameIndex) {
    auto& rightvalues = right.values;
    size_t base = left.values.size();
    values.insert(values.end(), rightvalues.begin(), rightvalues.end());
    for (auto& kv : right.nameIndex) nameIndex.try_emplace(kv.first, kv.second + base);
}
vector<shared_ptr<RuntimeValue>> TupleValue::Values() const { return values; }
optional<size_t> TupleValue::IndexByName(const string& name) const {
    auto iter = nameIndex.find(name);
    if (iter == nameIndex.end()) return {};
    return iter->second;
}
RuntimeValueResult TupleValue::ValueByName(const string& name) const {
    auto optindex = IndexByName(name);
    if (!optindex) return {};
    return values[*optindex];
}
RuntimeValueResult TupleValue::ValueByIndex(BigInt index) const {
    if (index <= 0 || index > values.size()) return {};
    return values[index.ClampToLong() - 1];
}
shared_ptr<runtime::Type> TupleValue::TypeOfValue() const { return make_shared<TupleType>(); }
RuntimeValueResult TupleValue::BinaryPlus(const RuntimeValue& other) const {
    auto p = dynamic_cast<const TupleValue*>(&other);
    if (!p) return {};
    return make_shared<TupleValue>(*this, *p);
}
RuntimeValueResult TupleValue::Field(const string& name) const { return ValueByName(name); }
RuntimeValueResult TupleValue::Field(const RuntimeValue& index) const {
    auto p = dynamic_cast<const IntegerValue*>(&index);
    if (!p) return {};
    return ValueByIndex(p->Value());
}
bool TupleValue::AssignNamedField(const string& name, const shared_ptr<RuntimeValue>& val) {
    auto optindex = IndexByName(name);
    if (!optindex) return false;
    values[*optindex] = val;
    return true;
}
bool TupleValue::AssignIndexedField(const BigInt& index, const shared_ptr<RuntimeValue>& val) {
    if (index <= 0 || index > values.size()) return false;
    values[index.ClampToLong() - 1] = val;
    return true;
}
void TupleValue::DoPrintSelf(ostream& out, set<shared_ptr<const RuntimeValue>>& recGuard) const {
    auto ins = recGuard.insert(shared_from_this());
    if (!ins.second) {
        out << "{...}";
        return;
    }
    out << "{\n";
    size_t n = values.size();
    vector<optional<string>> names(n);
    for (auto& kv : nameIndex) names[kv.second] = kv.first;
    stringstream indented;
    for (size_t i = 0; i < n; i++) {
        auto& optname = names[i];
        if (optname)
            indented << *optname;
        else
            indented << i + 1;
        indented << " := ";
        values[i]->DoPrintSelf(indented, recGuard);
        indented << '\n';
    }
    while (true) {
        string ln;
        if (!getline(indented, ln)) break;
        out << "    " << ln << '\n';
    }
    out << '}';
    recGuard.erase(ins.first);
}

StringSliceFunction::StringSliceFunction(const shared_ptr<const StringValue>& _this) : _this(_this) {}
RuntimeValueResult StringSliceFunction::Call(const vector<shared_ptr<RuntimeValue>>& args) const {
    if (args.size() != 3) return runtime_error("The string.Slice function requires 3 arguments that are integers");
    const IntegerValue* _args[3];
    ranges::transform(args, _args,
                      [](const shared_ptr<RuntimeValue>& val) { return dynamic_cast<const IntegerValue*>(val.get()); });
    bool types_ok = true;
    for (int i = 0; i < 3; i++) {
        if (!_args[i]) {
            types_ok = false;
            break;
        }
    }
    if (!types_ok) {
        stringstream msg("The string.Slice function expected \"int\" arguments, but ");
        bool first = true;
        const char* const argnames[] = {"start", "stop", "step"};
        for (int i = 0; i < 3; i++) {
            if (_args[i]) continue;
            if (!first) msg << "; ";
            first = false;
            msg << "argument" << i + 1 << " (" << argnames[i] << ") was \"" << args[i]->TypeOfValue()->Name() << '\"';
        }
        return runtime_error(msg.str());
    }
    if (!_args[2]->Value()) return runtime_error("The string.Slice function's third argument (step) cannot be 0");
    return make_shared<StringValue>(_this->Slice(_args[0]->Value(), _args[1]->Value(), _args[2]->Value()));
}
shared_ptr<runtime::Type> StringSliceFunction::TypeOfValue() const {
    return make_shared<FuncType>(false, vector<shared_ptr<Type>>(3, make_shared<IntegerType>()),
                                 make_shared<StringType>());
}
void StringSliceFunction::DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>&) const {
    out << "<built-in function string.Slice(start: int, stop: int, step: int) -> string>";
}

StringSplitFunction::StringSplitFunction(const shared_ptr<const StringValue>& _this) : _this(_this) {}
RuntimeValueResult StringSplitFunction::Call(const vector<shared_ptr<RuntimeValue>>& args) const {
    if (args.size() != 1) return runtime_error("The string.Split function accepts exactly 1 string argument");
    auto strval = dynamic_cast<const StringValue*>(args[0].get());
    if (!strval)
        return runtime_error("The string.Split function expected a string argument, but received \"" +
                             args[0]->TypeOfValue()->Name() + "\"");
    vector<shared_ptr<RuntimeValue>> strings;
    std::ranges::transform(_this->Split(strval->Value()), back_inserter(strings),
                           [](const string& val) { return make_shared<StringValue>(val); });
    return make_shared<ArrayValue>(strings);
}
shared_ptr<runtime::Type> StringSplitFunction::TypeOfValue() const {
    return make_shared<FuncType>(true, vector<shared_ptr<Type>>{make_shared<StringType>()}, make_shared<ArrayType>());
}
void StringSplitFunction::DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>&) const {
    out << "<built-in function string.Split(sep: string) -> []>";
}

StringSplitWSFunction::StringSplitWSFunction(const shared_ptr<const StringValue>& _this) : _this(_this) {}
RuntimeValueResult StringSplitWSFunction::Call(const vector<shared_ptr<RuntimeValue>>& args) const {
    if (args.size()) return runtime_error("The string.SplitWS function accepts no arguments");
    vector<shared_ptr<RuntimeValue>> strings;
    std::ranges::transform(_this->SplitWS(), back_inserter(strings),
                           [](const string& val) { return make_shared<StringValue>(val); });
    return make_shared<ArrayValue>(strings);
}
shared_ptr<runtime::Type> StringSplitWSFunction::TypeOfValue() const {
    return make_shared<FuncType>(true, vector<shared_ptr<Type>>(), make_shared<ArrayType>());
}
void StringSplitWSFunction::DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>&) const {
    out << "<built-in function string.SplitWS() -> []>";
}

StringJoinFunction::StringJoinFunction(const std::shared_ptr<const StringValue>& _this) : _this(_this) {}
RuntimeValueResult StringJoinFunction::Call(const std::vector<std::shared_ptr<RuntimeValue>>& args) const {
    if (args.size() != 1) return runtime_error("The string.Join function accepts exactly 1 array argument");
    auto arrval = dynamic_cast<const ArrayValue*>(args[0].get());
    if (!arrval)
        return runtime_error("The string.Join function expects an array of strings as the argument, but received \"" +
                             args[0]->TypeOfValue()->Name() + "\"");
    vector<const StringValue*> strvals;
    strvals.reserve(arrval->Value.size());
    std::ranges::transform(arrval->Value, back_inserter(strvals),
                           [](const std::pair<BigInt, std::shared_ptr<RuntimeValue>>& kv) {
                               return dynamic_cast<const StringValue*>(kv.second.get());
                           });
    for (auto i : strvals)
        if (!i) return runtime_error("The string.Join function received an array with non-string values");
    vector<string> strs(strvals.size());
    std::ranges::transform(strvals, strs.begin(), [](const StringValue* val) { return val->Value(); });
    return make_shared<StringValue>(_this->Join(strs));
}
std::shared_ptr<runtime::Type> StringJoinFunction::TypeOfValue() const {
    return make_shared<FuncType>(true, vector<shared_ptr<Type>>{make_shared<ArrayType>()}, make_shared<StringType>());
}
void StringJoinFunction::DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>&) const {
    out << "<built-in function string.Join(strings: []) -> string>";
}

}  // namespace runtime
