#include "interp/input.h"
using namespace std;

namespace interp {

std::shared_ptr<runtime::RuntimeValue> InputFunction::UserCall(
    RuntimeContext& context, const std::vector<std::shared_ptr<runtime::RuntimeValue>>& args) const {
    if (args.size()) {
        auto pos = context.CallStack.Top();
        context.CallStack.Pop();
        context.SetThrowingState(runtime::DRuntimeError("The input function accepts no arguments"), pos);
        context.CallStack.Push(pos);
        return nullptr;
    }
    string line;
    getline(*context.Input, line);
    return make_shared<runtime::StringValue>(line);
}

std::shared_ptr<runtime::FuncType> InputFunction::FunctionType() const {
    return make_shared<runtime::FuncType>(false, 0, make_shared<runtime::StringType>());
}

void InputFunction::DoPrintSelf(std::ostream& out, std::set<std::shared_ptr<const RuntimeValue>>& recGuard) const {
    out << "<built-in function input() -> string>";
}

}
