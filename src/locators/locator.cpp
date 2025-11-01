#include "locators/locator.h"

#include <sstream>
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

SpanLocator::SpanLocator(const std::shared_ptr<const CodeFile>& file, size_t pos, size_t length)
    : pos(pos), length(length), file(file) {}
SpanLocator::SpanLocator(const Locator& loc, size_t length) : SpanLocator(loc.File(), loc.Position(), length) {}
Locator SpanLocator::Start() const { return {file, pos}; }
Locator SpanLocator::End() const { return {file, pos + length}; }
size_t SpanLocator::Length() const { return length; }
std::string SpanLocator::Excerpt() const { return file->AllText().substr(pos, length); }
const shared_ptr<const CodeFile>& SpanLocator::File() const { return file; }

}  // namespace locators
