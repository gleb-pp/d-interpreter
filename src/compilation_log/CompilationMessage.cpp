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
    out << loc.FileName() << ":\n";
    auto [line, col] = loc.LineCol();
    string linenum = to_string(line + 1);
    int width = static_cast<int>(min(options.Width, static_cast<size_t>(numeric_limits<int>::max())));
    width -= static_cast<int>(linenum.size()) + 2;
    width = max(0, width);
    out << linenum << " |";
    size_t linewidth = loc.File()->LineLength(line);
    int toleft, toright;
    if (static_cast<long>(linewidth) <= width) {
        toleft = col;
        toright = linewidth - col;
    } else {
        toleft = width / 2;
        toright = width - toleft;
        if (toleft > static_cast<long>(col)) {
            toright += toleft - col;
            toleft = col;
        }
        if (static_cast<long>(col + toright) > static_cast<long>(linewidth)) {
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
void CompilationMessage::WriteContextToStream(ostream& out, const locators::SpanLocator& loc,
                                              [[maybe_unused]] const FormatOptions& options) {
    const locators::CodeFile& file = *loc.File();
    out << file.FileName() << ":\n";
    size_t startpos = loc.Start().Position(), endpos = loc.End().Position();
    if (endpos < startpos) swap(endpos, startpos);
    auto locstart = file.LineCol(startpos);
    auto locend = file.LineCol(endpos);
    if (locend.first > locstart.first && locend.second == 0) {
        locend.first--;
        locend.second = file.LineLength(locend.first);
    }
    locstart.first++;
    locend.first++;
    int linenum_chars = to_string(locend.first).length();
    for (size_t linenum = locstart.first; linenum <= locend.first; linenum++) {
        bool first = linenum == locstart.first;
        bool last = linenum == locend.first;
        {
            string linenum_str = to_string(linenum);
            out << linenum_str << string(linenum_chars - linenum_str.size() + 1, ' ') << '|';
        }
        string linetext = file.LineTextWithoutLineFeed(linenum - 1);
        out << linetext << '\n';
        out << string(linenum_chars + 1, ' ') << (last ? ' ' : '|');
        size_t hlstart = first ? locstart.second : 0;
        size_t hlend = last ? locend.second : linetext.size() + 1;  // +1 for the \n
        hlend += (hlstart == hlend);
        out << string(hlstart, ' ') << string(hlend - hlstart, '^') << '\n';
    }
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
