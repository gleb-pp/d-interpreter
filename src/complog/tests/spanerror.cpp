#include "spanerror.h"

#include "dinterp/complog/CompilationMessage.h"
using namespace std;
using namespace locators;
using namespace complog;

void SpanError::WriteMessageToStream(std::ostream& out,
                                     const complog::CompilationMessage::FormatOptions& options) const {
    out << "Span error\n";
}
std::vector<locators::Locator> SpanError::Locators() const { return {}; }
std::vector<locators::SpanLocator> SpanError::SpanLocators() const { return {loc}; }
SpanError::SpanError(const locators::SpanLocator& loc) : CompilationMessage(Severity::Error(), "SPANBAD"), loc(loc) {}
