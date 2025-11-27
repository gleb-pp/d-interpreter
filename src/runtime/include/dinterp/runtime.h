#include <variant>

#include "dinterp/runtime/derror.h"
#include "dinterp/runtime/types.h"
#include "dinterp/runtime/values.h"

namespace dinterp {
namespace runtime {

using TypeOrValue = std::variant<std::shared_ptr<Type>, std::shared_ptr<RuntimeValue>>;

}
}
