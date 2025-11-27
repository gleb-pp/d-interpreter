#include "baderror.h"

#include "dinterp/complog/CompilationMessage.h"
using namespace std;
using namespace dinterp;
using namespace locators;
using namespace complog;

void BadError::WriteMessageToStream(std::ostream& out,
                                    const complog::CompilationMessage::FormatOptions& options) const {
    out << "Bad error\n";
}
std::vector<locators::Locator> BadError::Locators() const { return {loc}; }
BadError::BadError(const locators::Locator& loc) : CompilationMessage(Severity::Error(), "BAD"), loc(loc) {}

void DoubleBadError::WriteMessageToStream(std::ostream& out,
                                          const complog::CompilationMessage::FormatOptions& options) const {
    out << "Very bad\n";
}
std::vector<locators::Locator> DoubleBadError::Locators() const { return {loc1, loc2}; }
DoubleBadError::DoubleBadError(const locators::Locator& loc1, const locators::Locator& loc2)
    : complog::CompilationMessage(Severity::Error(), "DBLBAD"), loc1(loc1), loc2(loc2) {}
