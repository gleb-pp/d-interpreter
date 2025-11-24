#include "dinterp/interp/userCallable.h"

namespace interp {

std::shared_ptr<runtime::Type> UserCallable::TypeOfValue() const { return FunctionType(); }

}  // namespace interp
