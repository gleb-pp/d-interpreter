#pragma once
#include <optional>

#include "dinterp/syntax.h"
#include "execution.h"
#include "runtimeContext.h"

namespace dinterp {
namespace interp {

void Run(interp::RuntimeContext& context, ast::Body& program);

}
}  // namespace dinterp
