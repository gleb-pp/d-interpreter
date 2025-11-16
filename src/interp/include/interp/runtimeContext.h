#pragma once
#include <iostream>
#include <variant>
#include <vector>

#include "locators/locator.h"
#include "runtime.h"
#include "runtime/derror.h"
#include "runtime/values.h"

namespace interp {

class CallStackTrace {
    std::vector<locators::SpanLocator> entries;

public:
    CallStackTrace(const std::vector<locators::SpanLocator>& entries);
    void WriteToStream(std::ostream& out, size_t maxentries) const;
};

class CallStack {
    std::vector<locators::SpanLocator> entries;

public:
    const size_t Capacity;
    CallStack(size_t capacity);
    bool Push(locators::SpanLocator position);
    void Pop();
    CallStackTrace Report(size_t entry_limit) const;
};

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

class RuntimeContext {
public:
    std::ostream* const Output;
    std::istream* const Input;
    interp::CallStack CallStack;
    const size_t StackTraceMaxEntries;
    RuntimeState State;
    RuntimeContext(std::istream& input, std::ostream& output, size_t callStackCapacity, size_t stackTraceMaxEntries);
};

}  // namespace interp
