#include "complog/CompilationLog.h"

#include <sstream>

#include "complog/CompilationMessage.h"
using namespace std;

namespace complog {
std::string AccumulatedCompilationLog::ToString(const CompilationMessage::FormatOptions& options) const {
    stringstream out;
    WriteToStream(out, options);
    return out.str();
}
void AccumulatedCompilationLog::WriteToStream(std::ostream& out,
                                              const CompilationMessage::FormatOptions& options) const {
    WriteToStream(out, {0}, options);
}
std::string AccumulatedCompilationLog::ToString(const Severity& leastLevel,
                                                const CompilationMessage::FormatOptions& options) const {
    stringstream out;
    WriteToStream(out, leastLevel, options);
    return out.str();
}
void AccumulatedCompilationLog::WriteToStream(std::ostream& out, const Severity& leastLevel,
                                              const CompilationMessage::FormatOptions& options) const {
    bool first = true;
    for (const auto& pmsg : messages) {
        if (pmsg->MessageSeverity() < leastLevel) continue;
        if (!first) out << '\n';
        pmsg->WriteToStream(out, options);
        first = false;
    }
}
void AccumulatedCompilationLog::Log(const std::shared_ptr<CompilationMessage>& message) { messages.push_back(message); }
const vector<shared_ptr<CompilationMessage>>& AccumulatedCompilationLog::Messages() const { return messages; }
StreamingCompilationLog::StreamingCompilationLog(std::ostream& out, const CompilationMessage::FormatOptions& options)
    : StreamingCompilationLog(out, options, {0}) {}
StreamingCompilationLog::StreamingCompilationLog(std::ostream& out, const CompilationMessage::FormatOptions& options,
                                                 Severity minSeverity)
    : out(&out), minSeverity(minSeverity), opts(options) {}
void StreamingCompilationLog::Log(const std::shared_ptr<CompilationMessage>& message) {
    if (message->MessageSeverity() < minSeverity) return;
    message->WriteToStream(*out, opts);
    out->put('\n');
}

CombinedCompilationLog::CombinedCompilationLog(const std::vector<std::shared_ptr<ICompilationLog>>& recipients)
    : logs(recipients) {}
CombinedCompilationLog::CombinedCompilationLog(
    const std::initializer_list<std::shared_ptr<ICompilationLog>>& recipients)
    : logs(std::move(recipients)) {}
void CombinedCompilationLog::Log(const std::shared_ptr<CompilationMessage>& message) {
    for (auto& ptr : logs) ptr->Log(message);
}
}  // namespace complog
