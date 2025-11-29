#pragma once
#include "dinterp/complog/CompilationMessage.h"
#include "dinterp/locators/locator.h"

class SpanError : public dinterp::complog::CompilationMessage {
private:
    dinterp::locators::SpanLocator loc;

protected:
    void WriteMessageToStream(std::ostream& out,
                              const dinterp::complog::CompilationMessage::FormatOptions& options) const override;
    std::vector<dinterp::locators::Locator> Locators() const override;
    std::vector<dinterp::locators::SpanLocator> SpanLocators() const override;

public:
    SpanError(const dinterp::locators::SpanLocator& loc);
};
