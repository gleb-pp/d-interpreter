#include <variant>

#include "runtime/derror.h"
#include "runtime/types.h"
#include "runtime/values.h"

namespace runtime {

using TypeOrValue = std::variant<std::shared_ptr<Type>, std::shared_ptr<RuntimeValue>>;

}
