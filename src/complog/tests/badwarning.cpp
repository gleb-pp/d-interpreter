#include "badwarning.h"

#include "dinterp/complog/CompilationMessage.h"
using namespace std;
using namespace dinterp;
using namespace locators;
using namespace complog;

void BadWarning::WriteMessageToStream(std::ostream& out,
                                      const complog::CompilationMessage::FormatOptions& options) const {
    out << "Bad warning\n";
}
std::vector<locators::Locator> BadWarning::Locators() const { return {loc}; }
BadWarning::BadWarning(const locators::Locator& loc) : CompilationMessage(Severity::Warning(), "BAD"), loc(loc) {}
