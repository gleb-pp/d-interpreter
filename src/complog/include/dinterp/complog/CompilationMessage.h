#pragma once
#include <ostream>
#include <string>

#include "dinterp/locators/locator.h"

namespace dinterp {
namespace complog {
class Severity {
private:
    static constexpr const char* SEVERITY_STR[] = {"[Info]", "[Warning]", "[Error]"};
    const int index;

public:
    Severity(int index);
    static Severity Info();
    static Severity Warning();
    static Severity Error();
    int Index() const;
    std::string ToString() const;
    bool operator<(const Severity& other) const;
    bool operator>(const Severity& other) const;
    bool operator<=(const Severity& other) const;
    bool operator>=(const Severity& other) const;
    bool operator==(const Severity& other) const;
    bool operator!=(const Severity& other) const;
};

class CompilationMessage {
public:
    struct FormatOptions {
        bool Context = false;
        size_t Width = 80;

        FormatOptions(bool context, size_t width);
        FormatOptions() = default;
        static FormatOptions All(size_t width);
        FormatOptions WithContext() const;
        FormatOptions WithoutContext() const;
        FormatOptions WithWidth(size_t width) const;
    };

private:
    const Severity severity;
    const std::string code;

protected:
    // Example output:
    //
    // program.pas:
    // 16:   for i := 1 to High(s) do
    //                          ^
    static void WriteContextToStream(std::ostream& out, const locators::Locator& loc, const FormatOptions& options);
    static void WriteContextToStream(std::ostream& out, const locators::SpanLocator& loc, const FormatOptions& options);

public:
    CompilationMessage(Severity severity, const std::string& code);
    const std::string& Code() const;
    const Severity MessageSeverity() const;
    virtual void WriteMessageToStream(std::ostream& out, const FormatOptions& options) const = 0;
    virtual std::vector<locators::Locator> Locators() const = 0;
    virtual std::vector<locators::SpanLocator> SpanLocators() const;
    std::string ToString(const FormatOptions& options);
    // Writes:
    //
    // `[Severity] (Code) WriteMessageToStream`
    //
    // `WriteContextToStream for all locators in Locators()`
    //
    // Derived classes must define `WriteMessageToStream`.
    void WriteToStream(std::ostream& out, const FormatOptions& options);
    virtual ~CompilationMessage() = default;
};
}  // namespace complog
}
