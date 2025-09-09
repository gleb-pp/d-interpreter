#pragma once
#include <memory>

#include "CodeFile.h"

namespace locators {
class Locator {
    const size_t pos;
    const std::shared_ptr<const CodeFile> file;

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
};
}  // namespace locators
