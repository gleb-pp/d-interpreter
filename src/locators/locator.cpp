#include "locators/locator.h"

#include <limits>
#include <sstream>
#include <stdexcept>
using namespace std;

namespace locators {
Locator::Locator(const shared_ptr<const CodeFile>& file, size_t pos) : pos(pos), file(file) {}
std::string Locator::Pretty() const {
    stringstream res;
    auto [line, col] = LineCol();
    res << file->FileName() << ':' << line + 1 << ':' << col;
    return res.str();
}
size_t Locator::Position() const { return pos; }
size_t Locator::Line() const { return file->Line(pos); }
size_t Locator::Column() const { return file->Column(pos); }
pair<size_t, size_t> Locator::LineCol() const { return file->LineCol(pos); }
const std::string& Locator::FileName() const { return file->FileName(); }
CodeContext Locator::Context(size_t toleft, size_t toright) const { return file->Context(pos, toleft, toright); }
const shared_ptr<const CodeFile>& Locator::File() const { return file; }

void Locator::WritePrettyExcerpt(ostream& out, size_t suggested_width) const {
    out << FileName() << ":\n";
    auto [line, col] = LineCol();
    string linenum = to_string(line + 1);
    int width = static_cast<int>(min(suggested_width, static_cast<size_t>(numeric_limits<int>::max())));
    width -= static_cast<int>(linenum.size()) + 2;
    width = max(0, width);
    out << linenum << " |";
    size_t linewidth = file->LineLength(line);
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
    auto ctx = Context(static_cast<size_t>(toleft), static_cast<size_t>(toright));
    out << ctx.Text << '\n';
    out << string(linenum.size() + 2 + ctx.PointerWithinText, ' ') << "^\n";
}

SpanLocator::SpanLocator(const std::shared_ptr<const CodeFile>& file, size_t pos, size_t length)
    : pos(pos), length(length), file(file) {}
SpanLocator::SpanLocator(const SpanLocator& a, const SpanLocator& b) : file(a.file) {
    if (a.file != b.file)
        throw std::runtime_error("Tried to merge two spans from different files: " + a.file->FileName() + " and " +
                                 b.file->FileName());
    pos = min(a.pos, b.pos);
    size_t end = max(a.pos + a.length, b.pos + b.length);
    length = end - pos;
}
SpanLocator::SpanLocator(const Locator& loc, size_t length) : SpanLocator(loc.File(), loc.Position(), length) {}
string SpanLocator::Pretty() const {
    stringstream res;
    auto [sline, scol] = file->LineCol(pos);
    auto [eline, ecol] = file->LineCol(pos + length);
    res << file->FileName() << ':' << sline + 1 << ':' << scol << "--" << eline + 1 << ':' << ecol;
    return res.str();
}
Locator SpanLocator::Start() const { return {file, pos}; }
Locator SpanLocator::End() const { return {file, pos + length}; }
size_t SpanLocator::Length() const { return length; }
std::string SpanLocator::Excerpt() const { return file->AllText().substr(pos, length); }
const shared_ptr<const CodeFile>& SpanLocator::File() const { return file; }

void SpanLocator::WritePrettyExcerpt(ostream& out, [[maybe_unused]] size_t suggested_width) const {
    out << file->FileName() << ":\n";
    size_t endpos = pos + length;
    auto locstart = file->LineCol(pos);
    auto locend = file->LineCol(endpos);
    if (locend.first > locstart.first && locend.second == 0) {
        locend.first--;
        locend.second = file->LineLength(locend.first);
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
        string linetext = file->LineTextWithoutLineFeed(linenum - 1);
        out << linetext << '\n';
        out << string(linenum_chars + 1, ' ') << (last ? ' ' : '|');
        size_t hlstart = first ? locstart.second : 0;
        size_t hlend = last ? locend.second : linetext.size() + 1;  // +1 for the \n
        hlend += (hlstart == hlend);
        out << string(hlstart, ' ') << string(hlend - hlstart, '^') << '\n';
    }
}

}  // namespace locators
