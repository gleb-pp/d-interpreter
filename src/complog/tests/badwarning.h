#pragma once
#include "dinterp/complog/CompilationMessage.h"
#include "dinterp/locators/locator.h"

class BadWarning : public dinterp::complog::CompilationMessage {
private:
    dinterp::locators::Locator loc;

protected:
    void WriteMessageToStream(std::ostream& out,
                              const dinterp::complog::CompilationMessage::FormatOptions& options) const override;
    std::vector<dinterp::locators::Locator> Locators() const override;

public:
    BadWarning(const dinterp::locators::Locator& loc);
};
