#include "complog/CompilationMessage.h"

#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

#include "locators/locator.h"
using namespace std;

namespace complog {
Severity::Severity(int index) : index(index) {
    if (index < 0 || static_cast<size_t>(index) >= sizeof(SEVERITY_STR) / sizeof(SEVERITY_STR[0]))
        throw runtime_error("Index " + to_string(index) + " does not correspond to a severity level");
}
Severity Severity::Info() { return {0}; }
Severity Severity::Warning() { return {1}; }
Severity Severity::Error() { return {2}; }
int Severity::Index() const { return index; }
string Severity::ToString() const { return SEVERITY_STR[index]; }
bool Severity::operator<(const Severity& other) const { return index < other.index; }
bool Severity::operator>(const Severity& other) const { return index > other.index; }
bool Severity::operator<=(const Severity& other) const { return index <= other.index; }
bool Severity::operator>=(const Severity& other) const { return index >= other.index; }
bool Severity::operator==(const Severity& other) const { return index == other.index; }
bool Severity::operator!=(const Severity& other) const { return index != other.index; }

CompilationMessage::FormatOptions::FormatOptions(bool context, size_t width) : Context(context), Width(width) {}
CompilationMessage::FormatOptions CompilationMessage::FormatOptions::All(size_t width) {
    return CompilationMessage::FormatOptions(true, width);
}
CompilationMessage::FormatOptions CompilationMessage::FormatOptions::WithContext() const {
    auto res = *this;
    res.Context = true;
    return res;
}
CompilationMessage::FormatOptions CompilationMessage::FormatOptions::WithoutContext() const {
    auto res = *this;
    res.Context = false;
    return res;
}
CompilationMessage::FormatOptions CompilationMessage::FormatOptions::WithWidth(size_t width) const {
    auto res = *this;
    res.Width = width;
    return res;
}

vector<locators::SpanLocator> CompilationMessage::SpanLocators() const { return {}; }
void CompilationMessage::WriteContextToStream(ostream& out, const locators::Locator& loc,
                                              const FormatOptions& options) {
    loc.WritePrettyExcerpt(out, options.Width);
}

void CompilationMessage::WriteContextToStream(ostream& out, const locators::SpanLocator& loc,
                                              [[maybe_unused]] const FormatOptions& options) {
    loc.WritePrettyExcerpt(out, options.Width);
}

CompilationMessage::CompilationMessage(Severity severity, const string& code) : severity(severity), code(code) {}
const string& CompilationMessage::Code() const { return code; }
const Severity CompilationMessage::MessageSeverity() const { return severity; }
string CompilationMessage::ToString(const FormatOptions& options) {
    stringstream out;
    WriteToStream(out, options);
    return out.str();
}
void CompilationMessage::WriteToStream(ostream& out, const FormatOptions& options) {
    out << severity.ToString() << " (" << code << ") ";
    WriteMessageToStream(out, options);
    if (options.Context) {
        for (const locators::Locator& loc : Locators()) WriteContextToStream(out, loc, options);
        for (const locators::SpanLocator& loc : SpanLocators()) WriteContextToStream(out, loc, options);
    }
}
}  // namespace complog
