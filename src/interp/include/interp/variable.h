#pragma once
#include <memory>
#include <string>

#include "runtime.h"
#include "runtime/values.h"

namespace interp {

class Variable : public std::enable_shared_from_this<Variable> {
    std::string name;
    std::shared_ptr<runtime::RuntimeValue> val;

public:
    Variable(const std::shared_ptr<runtime::RuntimeValue>& content);
    Variable();  // contains None
    const std::string& Name() const;
    void Assign(const std::shared_ptr<runtime::RuntimeValue>& content);
    const std::shared_ptr<runtime::RuntimeValue>& Content() const;
};

}  // namespace interp
