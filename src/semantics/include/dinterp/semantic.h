#pragma once
#include "complog/CompilationLog.h"
#include "syntax.h"

namespace semantic {

bool Analyze(complog::ICompilationLog& log, const std::shared_ptr<ast::Body>& program);

}
