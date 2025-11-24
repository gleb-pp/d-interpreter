#pragma once
#include "complog/CompilationMessage.h"
#include "locators/locator.h"

class SpanError : public complog::CompilationMessage {
private:
    locators::SpanLocator loc;

protected:
    void WriteMessageToStream(std::ostream& out,
                              const complog::CompilationMessage::FormatOptions& options) const override;
    std::vector<locators::Locator> Locators() const override;
    std::vector<locators::SpanLocator> SpanLocators() const override;

public:
    SpanError(const locators::SpanLocator& loc);
};
