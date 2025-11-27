#pragma once
#include <string>
#include <vector>

namespace dinterp {
namespace locators {
struct CodeContext {
    std::string Text;
    size_t PointerWithinText;
    CodeContext(const std::string& text, size_t pointerPositionWithinText);
};

class CodeFile {
    const std::string filename;
    const std::string content;
    const std::vector<size_t> eolns;
    CodeFile(const std::string& filename, const std::string& content, const std::vector<size_t> eolns);
    static std::vector<size_t> FindEolns(const std::string& text);

public:
    CodeFile(const std::string& filename, const std::string& content);
    std::pair<size_t, size_t> LineCol(size_t pos) const;
    size_t Line(size_t pos) const;
    size_t Column(size_t pos) const;
    size_t Position(size_t line, size_t col) const;
    size_t LineStartPosition(size_t line) const;
    const std::string& FileName() const;
    CodeContext Context(size_t pos, size_t toleft, size_t toright) const;
    size_t LineLength(size_t line) const;
    size_t LineCount() const;
    const std::string& AllText() const;
    std::string LineTextWithoutLineFeed(size_t line) const;
};
}  // namespace locators
}  // namespace dinterp
