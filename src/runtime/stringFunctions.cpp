#include <algorithm>
#include <sstream>

#include "dinterp/runtime/values.h"
using namespace std;

namespace dinterp {
namespace runtime {
StringSliceFunction::StringSliceFunction(const shared_ptr<const StringValue>& _this) : _this(_this) {}
RuntimeValueResult StringSliceFunction::Call(const vector<shared_ptr<RuntimeValue>>& args) const {
    if (args.size() != 3) return DRuntimeError("The string.Slice function requires 3 arguments that are integers");
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
        stringstream msg;
        msg << "The string.Slice function expected \"int\" arguments, but ";
        bool first = true;
        const char* const argnames[] = {"start", "stop", "step"};
        for (int i = 0; i < 3; i++) {
            if (_args[i]) continue;
            if (!first) msg << "; ";
            first = false;
            msg << "argument " << i + 1 << " (" << argnames[i] << ") was \"" << args[i]->TypeOfValue()->Name() << '\"';
        }
        return DRuntimeError(msg.str());
    }
    if (!_args[2]->Value()) return DRuntimeError("The string.Slice function's third argument (step) cannot be 0");
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
    if (args.size() != 1) return DRuntimeError("The string.Split function accepts exactly 1 string argument");
    auto strval = dynamic_cast<const StringValue*>(args[0].get());
    if (!strval)
        return DRuntimeError("The string.Split function expected a string argument, but received \"" +
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
    if (args.size()) return DRuntimeError("The string.SplitWS function accepts no arguments");
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
    if (args.size() != 1) return DRuntimeError("The string.Join function accepts exactly 1 array argument");
    auto arrval = dynamic_cast<const ArrayValue*>(args[0].get());
    if (!arrval)
        return DRuntimeError("The string.Join function expects an array of strings as the argument, but received \"" +
                             args[0]->TypeOfValue()->Name() + "\"");
    vector<const StringValue*> strvals;
    strvals.reserve(arrval->Value.size());
    std::ranges::transform(arrval->Value, back_inserter(strvals),
                           [](const std::pair<BigInt, std::shared_ptr<RuntimeValue>>& kv) {
                               return dynamic_cast<const StringValue*>(kv.second.get());
                           });
    for (auto i : strvals)
        if (!i) return DRuntimeError("The string.Join function received an array with non-string values");
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
}  // namespace dinterp
