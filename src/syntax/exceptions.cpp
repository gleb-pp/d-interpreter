#include "dinterp/syntax.h"
using namespace std;

WrongNumberOfOperatorsSupplied::WrongNumberOfOperatorsSupplied(const std::string& astclass, size_t operandscount,
                                                               size_t operatorscount)
    : std::invalid_argument("\"" + astclass + "\"'s constructor received " + to_string(operandscount) +
                            " operands and " + to_string(operatorscount) + " operators.") {}
