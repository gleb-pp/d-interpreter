#pragma once
#include <optional>

#include "execution.h"
#include "runtimeContext.h"
#include "syntax.h"

namespace interp {

void Run(interp::RuntimeContext& context, ast::Body& program);

}
