#include "runtime/types.h"
#include "runtime/values.h"
#include <variant>

namespace runtime {

using TypeOrValue = std::variant<std::shared_ptr<Type>, std::shared_ptr<RuntimeValue>>;

}
