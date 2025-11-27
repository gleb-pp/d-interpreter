#pragma once
#include <memory>

#include "CodeFile.h"

namespace dinterp {
namespace locators {
class Locator {
    size_t pos;
    std::shared_ptr<const CodeFile> file;

public:
    Locator(const std::shared_ptr<const CodeFile>& file, size_t pos);
    std::string Pretty() const;
    size_t Position() const;
    size_t Line() const;
    size_t Column() const;
    std::pair<size_t, size_t> LineCol() const;
    const std::string& FileName() const;
    CodeContext Context(size_t toleft, size_t toright) const;
    const std::shared_ptr<const CodeFile>& File() const;
    void WritePrettyExcerpt(std::ostream& out, size_t suggested_width) const;
};

class SpanLocator {
    size_t pos, length;
    std::shared_ptr<const CodeFile> file;

public:
    SpanLocator(const std::shared_ptr<const CodeFile>& file, size_t pos, size_t length);
    SpanLocator(const SpanLocator& a, const SpanLocator& b);
    SpanLocator(const Locator& loc, size_t length);
    std::string Pretty() const;
    Locator Start() const;
    Locator End() const;
    size_t Length() const;
    std::string Excerpt() const;
    const std::shared_ptr<const CodeFile>& File() const;
    void WritePrettyExcerpt(std::ostream& out, size_t suggested_width) const;
};
}  // namespace locators
}
