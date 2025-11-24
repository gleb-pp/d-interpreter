#include "dinterp/runtime/derror.h"
using namespace std;

namespace runtime {

DRuntimeError::DRuntimeError(std::string message) : msg(message) {}
const char* DRuntimeError::what() const noexcept { return msg.c_str(); }

}  // namespace runtime
