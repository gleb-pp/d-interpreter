#include "locators/CodeFile.h"

#include <algorithm>
using namespace std;

namespace locators {
CodeContext::CodeContext(const string& text, size_t pointerPositionWithinText)
    : Text(text), PointerWithinText(pointerPositionWithinText) {}

CodeFile::CodeFile(const string& filename, const string& content, const vector<size_t> eolns)
    : filename(filename), content(content), eolns(eolns) {}
vector<size_t> CodeFile::FindEolns(const string& text) {
    vector<size_t> res;
    size_t i = 0;
    while (true) {
        i = text.find('\n', i);
        if (i == string::npos) break;
        res.push_back(i++);
    }
    return res;
}
CodeFile::CodeFile(const string& filename, const string& content) : CodeFile(filename, content, FindEolns(content)) {}
pair<size_t, size_t> CodeFile::LineCol(size_t pos) const {
    size_t line = Line(pos);
    return {line, pos - LineStartPosition(line)};
}
size_t CodeFile::Line(size_t pos) const { return lower_bound(eolns.begin(), eolns.end(), pos) - eolns.begin(); }
size_t CodeFile::Column(size_t pos) const { return pos - LineStartPosition(Line(pos)); }
size_t CodeFile::Position(size_t line, size_t col) const { return LineStartPosition(line) + col; }
size_t CodeFile::LineStartPosition(size_t line) const {
    if (line) return eolns[line - 1] + 1;
    return 0;
}
const string& CodeFile::FileName() const { return filename; }
CodeContext CodeFile::Context(size_t pos, size_t toleft, size_t toright) const {
    auto [line, col] = LineCol(pos);
    size_t linelen = LineLength(line);
    toleft = min(toleft, col);
    toright = min(toright, linelen - col);
    return {content.substr(pos - toleft, toleft + toright), toleft};
}
size_t CodeFile::LineLength(size_t line) const {
    return (line == eolns.size() ? content.size() : eolns[line]) - LineStartPosition(line);
}
size_t CodeFile::LineCount() const { return eolns.size() + 1; }
const string& CodeFile::AllText() const { return content; }
}  // namespace locators
