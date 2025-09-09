#include "complog/CompilationMessage.h"

#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
using namespace std;

namespace complog {
Severity::Severity(int index) : index(index) {
    if (index < 0 || index >= sizeof(SEVERITY_STR) / sizeof(SEVERITY_STR[0]))
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

void CompilationMessage::WriteContextToStream(ostream& out, const locators::Locator& loc,
                                              const FormatOptions& options) {
    out << loc.FileName() << ":\n";
    auto [line, col] = loc.LineCol();
    string linenum = to_string(line + 1);
    int width = static_cast<int>(min(options.Width, static_cast<size_t>(numeric_limits<int>::max())));
    width -= static_cast<int>(linenum.size()) + 2;
    width = max(0, width);
    out << linenum << " |";
    int linewidth = loc.File()->LineLength(line);
    int toleft, toright;
    if (linewidth <= width) {
        toleft = col;
        toright = linewidth - col;
    } else {
        toleft = width / 2;
        toright = width - toleft;
        if (toleft > col) {
            toright += toleft - col;
            toleft = col;
        }
        if (col + toright > linewidth) {
            toleft += col + toright - linewidth;
            toright = linewidth - col;
        }
    }
    toleft = max(0, toleft);
    toright = max(0, toright);
    auto ctx = loc.Context(static_cast<size_t>(toleft), static_cast<size_t>(toright));
    out << ctx.Text << '\n';
    out << string(linenum.size() + 2 + ctx.PointerWithinText, ' ') << "^\n";
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
    if (options.Context)
        for (const locators::Locator& loc : Locators()) WriteContextToStream(out, loc, options);
}
}  // namespace complog
