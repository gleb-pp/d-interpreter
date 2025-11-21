#include "semantic.h"

#include "runtime/types.h"
#include "semantic/statementChecker.h"
#include "semantic/valueTimeline.h"
using namespace std;

bool semantic::Analyze(complog::ICompilationLog& log, const shared_ptr<ast::Body>& program) {
    ValueTimeline tl;
    tl.StartScope();
    {
        auto zeroLoc = locators::SpanLocator(program->pos.File(), 0, 0);
        tl.Declare("input", zeroLoc);
        tl.AssignType("input", make_shared<runtime::FuncType>(false, 0, make_shared<runtime::StringType>()), zeroLoc);
        tl.LookupVariable("input");
    }
    StatementChecker chk(log, tl, false, false);
    program->AcceptVisitor(chk);

    if (chk.Terminated() == StatementChecker::TerminationKind::Errored) return false;
    return true;
}
