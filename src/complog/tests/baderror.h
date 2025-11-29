#pragma once
#include "dinterp/complog/CompilationMessage.h"
#include "dinterp/locators/locator.h"

class BadError : public dinterp::complog::CompilationMessage {
private:
    dinterp::locators::Locator loc;

protected:
    void WriteMessageToStream(std::ostream& out,
                              const dinterp::complog::CompilationMessage::FormatOptions& options) const override;
    std::vector<dinterp::locators::Locator> Locators() const override;

public:
    BadError(const dinterp::locators::Locator& loc);
};

class DoubleBadError : public dinterp::complog::CompilationMessage {
private:
    const dinterp::locators::Locator loc1, loc2;

protected:
    void WriteMessageToStream(std::ostream& out,
                              const dinterp::complog::CompilationMessage::FormatOptions& options) const override;
    std::vector<dinterp::locators::Locator> Locators() const override;

public:
    DoubleBadError(const dinterp::locators::Locator& loc1, const dinterp::locators::Locator& loc2);
};
