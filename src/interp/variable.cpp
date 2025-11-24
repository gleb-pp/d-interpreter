#include "interp/variable.h"

#include "runtime/values.h"
using namespace std;

namespace interp {

Variable::Variable(const string& name, const shared_ptr<runtime::RuntimeValue>& content) : name(name), val(content) {}

Variable::Variable(const string& name) : Variable(name, make_shared<runtime::NoneValue>()) {}

const string& Variable::Name() const { return name; }

void Variable::Assign(const shared_ptr<runtime::RuntimeValue>& content) { val = content; }

const shared_ptr<runtime::RuntimeValue>& Variable::Content() const { return val; }

}  // namespace interp
