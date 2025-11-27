#pragma once
#include <memory>
#include <vector>

#include "CompilationMessage.h"

namespace dinterp {
namespace complog {
class ICompilationLog {
public:
    virtual void Log(const std::shared_ptr<CompilationMessage>& message) = 0;
    virtual ~ICompilationLog() = default;
};

class StreamingCompilationLog : public ICompilationLog {
    std::ostream* out;
    Severity minSeverity;
    CompilationMessage::FormatOptions opts;

public:
    StreamingCompilationLog(std::ostream& out, const CompilationMessage::FormatOptions& options);
    StreamingCompilationLog(std::ostream& out, const CompilationMessage::FormatOptions& options, Severity minSeverity);
    void Log(const std::shared_ptr<CompilationMessage>& message) override;
    virtual ~StreamingCompilationLog() override = default;
};

class AccumulatedCompilationLog : public ICompilationLog {
    std::vector<std::shared_ptr<CompilationMessage>> messages;

public:
    std::string ToString(const CompilationMessage::FormatOptions& options) const;
    void WriteToStream(std::ostream& out, const CompilationMessage::FormatOptions& options) const;
    std::string ToString(const Severity& leastLevel, const CompilationMessage::FormatOptions& options) const;
    void WriteToStream(std::ostream& out, const Severity& leastLevel,
                       const CompilationMessage::FormatOptions& options) const;
    const std::vector<std::shared_ptr<CompilationMessage>>& Messages() const;
    void Log(const std::shared_ptr<CompilationMessage>& message) override;
    virtual ~AccumulatedCompilationLog() override = default;
};

class CombinedCompilationLog : public ICompilationLog {
    const std::vector<std::shared_ptr<ICompilationLog>> logs;

public:
    CombinedCompilationLog(const std::vector<std::shared_ptr<ICompilationLog>>& recipients);
    CombinedCompilationLog(const std::initializer_list<std::shared_ptr<ICompilationLog>>& recipients);
    void Log(const std::shared_ptr<CompilationMessage>& message) override;
    virtual ~CombinedCompilationLog() override = default;
};
}  // namespace complog
}
