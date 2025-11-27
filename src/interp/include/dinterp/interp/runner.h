#pragma once
#include <optional>

#include "execution.h"
#include "runtimeContext.h"
#include "dinterp/syntax.h"

namespace dinterp {
namespace interp {

void Run(interp::RuntimeContext& context, ast::Body& program);

}
}
