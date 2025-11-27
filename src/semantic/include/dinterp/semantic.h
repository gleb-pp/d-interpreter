#pragma once
#include "dinterp/complog/CompilationLog.h"
#include "dinterp/syntax.h"

namespace dinterp {
namespace semantic {

bool Analyze(complog::ICompilationLog& log, const std::shared_ptr<ast::Body>& program);

}
}  // namespace dinterp
