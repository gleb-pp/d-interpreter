#include "semantic.h"

#include "semantic/statementChecker.h"
#include "semantic/valueTimeline.h"
using namespace std;

bool semantic::Analyze(complog::ICompilationLog& log, const shared_ptr<ast::Body>& program) {
    ValueTimeline tl;
    StatementChecker chk(log, tl, false, false);
    program->AcceptVisitor(chk);

    if (chk.Terminated() == StatementChecker::TerminationKind::Errored) return false;
    return true;
}
