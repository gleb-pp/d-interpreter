#pragma once
#include <memory>
#include <string>

#include "dinterp/runtime.h"
#include "dinterp/runtime/values.h"

namespace dinterp {
namespace interp {

class Variable : public std::enable_shared_from_this<Variable> {
    std::string name;
    std::shared_ptr<runtime::RuntimeValue> val;

public:
    Variable(const std::string& name, const std::shared_ptr<runtime::RuntimeValue>& content);
    Variable(const std::string& name);  // contains None
    const std::string& Name() const;
    void Assign(const std::shared_ptr<runtime::RuntimeValue>& content);
    const std::shared_ptr<runtime::RuntimeValue>& Content() const;
};

}  // namespace interp
}
