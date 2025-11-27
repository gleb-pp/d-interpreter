#pragma once
#include <exception>
#include <string>

namespace dinterp {
namespace runtime {

class DRuntimeError : public std::exception {
    std::string msg;

public:
    DRuntimeError(std::string message);
    const char* what() const noexcept override;
    virtual ~DRuntimeError() = default;
};

}  // namespace runtime
}  // namespace dinterp
