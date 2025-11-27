#include "dinterp/interp/runtimeContext.h"

#include "dinterp/locators/locator.h"
using namespace std;

namespace dinterp {
namespace interp {

// CallStackTrace

CallStackTrace::CallStackTrace(const std::vector<locators::SpanLocator>& entries)
    : entries(entries), skippingSep(0), skipped(0) {}

CallStackTrace::CallStackTrace(const std::vector<locators::SpanLocator>& entries, size_t skippingSep, size_t skipped)
    : entries(entries), skippingSep(skippingSep), skipped(skipped) {}

static void WriteCallsInRange(ostream& out, vector<locators::SpanLocator>::const_iterator start,
                              vector<locators::SpanLocator>::const_iterator end) {
    bool first = true;
    for (auto iter = start; iter != end; ++iter) {
        if (!first) out << '\n';
        first = false;
        iter->WritePrettyExcerpt(out, 100);
    }
}

void CallStackTrace::WriteToStream(std::ostream& out) const {
    size_t n = entries.size();
    auto begin = entries.begin(), end = entries.end();
    if (skippingSep == 0 || skippingSep >= n) {
        WriteCallsInRange(out, begin, end);
        return;
    }
    auto mid = begin + skippingSep;
    WriteCallsInRange(out, begin, mid);
    out << "\nSkipping " << skipped << " calls...\n\n";
    WriteCallsInRange(out, mid, end);
}

// CallStack

CallStack::CallStack(size_t capacity) : Capacity(capacity) {}

bool CallStack::Push(locators::SpanLocator position) {
    if (entries.size() >= Capacity) return false;
    entries.push_back(position);
    return true;
}

void CallStack::Pop() { entries.pop_back(); }

locators::SpanLocator CallStack::Top() const { return entries.back(); }

CallStackTrace CallStack::Report(size_t entry_limit) const {
    size_t n = entries.size();
    if (n <= entry_limit) return {entries};
    size_t firsthalf = entry_limit / 2;
    size_t secondhalf = entry_limit - firsthalf;
    vector<locators::SpanLocator> locs;
    locs.reserve(entry_limit);
    locs.insert(locs.end(), entries.begin(), entries.begin() + firsthalf);
    locs.insert(locs.end(), entries.end() - secondhalf, entries.end());
    return {locs, firsthalf, n - entry_limit};
}

// RuntimeState

/*
struct RuntimeState {
public:
    struct Running {};
    struct Exiting {};
    struct Returning {
        std::shared_ptr<runtime::RuntimeValue> Value;
        Returning(const std::shared_ptr<runtime::RuntimeValue>& value);
    };
    struct Throwing {
        runtime::DRuntimeError Error;
        locators::SpanLocator Position;
        CallStackTrace StackTrace;
        Throwing(const runtime::DRuntimeError& error, const locators::SpanLocator& pos, const CallStackTrace& trace);
    };
    enum class Kind { Running, Exiting, Returning, Throwing };

private:
    std::variant<Running, Exiting, Returning, Throwing> state;

public:
    RuntimeState(const Running&);
    RuntimeState(const Exiting&);
    RuntimeState(const Returning& value);
    RuntimeState(const Throwing& error);
    RuntimeState& operator=(const RuntimeState& other) = default;
    RuntimeState& operator=(const Running&);
    RuntimeState& operator=(const Exiting&);
    RuntimeState& operator=(const Returning& value);
    RuntimeState& operator=(const Throwing& error);
    bool IsRunning() const;
    bool IsExiting() const;
    bool IsReturning() const;
    bool IsThrowing() const;
    Kind StateKind() const;
    const std::shared_ptr<runtime::RuntimeValue> GetReturnValue();
    const Throwing& GetError() const;
};
*/

RuntimeState::Returning::Returning(const std::shared_ptr<runtime::RuntimeValue>& value) : Value(value) {}
RuntimeState::Throwing::Throwing(const runtime::DRuntimeError& error, const locators::SpanLocator& pos,
                                 const CallStackTrace& trace)
    : Error(error), Position(pos), StackTrace(trace) {}

RuntimeState::RuntimeState(const Running& val) : state(val) {}

RuntimeState::RuntimeState(const Exiting& val) : state(val) {}

RuntimeState::RuntimeState(const Returning& value) : state(value) {}

RuntimeState::RuntimeState(const Throwing& error) : state(error) {}

RuntimeState& RuntimeState::operator=(const Running&) {
    state.emplace<Running>();
    return *this;
}

RuntimeState& RuntimeState::operator=(const Exiting&) {
    state.emplace<Exiting>();
    return *this;
}

RuntimeState& RuntimeState::operator=(const Returning& value) {
    state.emplace<Returning>(value);
    return *this;
}

RuntimeState& RuntimeState::operator=(const Throwing& error) {
    state.emplace<Throwing>(error);
    return *this;
}

bool RuntimeState::IsRunning() const { return state.index() == 0; }

bool RuntimeState::IsExiting() const { return state.index() == 1; }

bool RuntimeState::IsReturning() const { return state.index() == 2; }

bool RuntimeState::IsThrowing() const { return state.index() == 3; }

RuntimeState::Kind RuntimeState::StateKind() const { return static_cast<Kind>(state.index()); }

const std::shared_ptr<runtime::RuntimeValue>& RuntimeState::GetReturnValue() const {
    return get<Returning>(state).Value;
}

const RuntimeState::Throwing& RuntimeState::GetError() const { return get<Throwing>(state); }

// RuntimeContext

RuntimeContext::RuntimeContext(std::istream& input, std::ostream& output, size_t callStackCapacity,
                               size_t stackTraceMaxEntries)
    : Output(&output),
      Input(&input),
      Stack(callStackCapacity),
      StackTraceMaxEntries(stackTraceMaxEntries),
      State(RuntimeState::Running()) {}

CallStackTrace RuntimeContext::MakeStackTrace() const { return this->Stack.Report(StackTraceMaxEntries); }

void RuntimeContext::SetThrowingState(const runtime::DRuntimeError& error, const locators::SpanLocator& pos) {
    State = RuntimeState::Throwing(error, pos, MakeStackTrace());
}

}  // namespace interp
}  // namespace dinterp
