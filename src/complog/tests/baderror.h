#pragma once
#include "complog/CompilationMessage.h"
#include "locators/locator.h"

class BadError : public complog::CompilationMessage {
private:
    locators::Locator loc;

protected:
    void WriteMessageToStream(std::ostream& out,
                              const complog::CompilationMessage::FormatOptions& options) const override;
    std::vector<locators::Locator> Locators() const override;

public:
    BadError(const locators::Locator& loc);
};

class DoubleBadError : public complog::CompilationMessage {
private:
    const locators::Locator loc1, loc2;

protected:
    void WriteMessageToStream(std::ostream& out,
                              const complog::CompilationMessage::FormatOptions& options) const override;
    std::vector<locators::Locator> Locators() const override;

public:
    DoubleBadError(const locators::Locator& loc1, const locators::Locator& loc2);
};
