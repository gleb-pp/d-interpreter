#pragma once
#include "complog/CompilationMessage.h"
#include "locators/locator.h"

class BadWarning : public complog::CompilationMessage {
private:
    locators::Locator loc;

protected:
    void WriteMessageToStream(std::ostream& out,
                              const complog::CompilationMessage::FormatOptions& options) const override;
    std::vector<locators::Locator> Locators() const override;

public:
    BadWarning(const locators::Locator& loc);
};
