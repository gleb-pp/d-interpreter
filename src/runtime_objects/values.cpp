#include "runtime/values.h"

namespace runtime {

/*
 * If a function accepts an index, it is 1-based (the first element has index 1)
 */

// Implement class RuntimeValue

// Implement class IntegerValue
/*
 * Has fields:
 * + Round: int = this
 * + Floor: int = this
 * + Ceil: int = this
 * + Frac: real = 0.0
 */

// Implement class RealValue
/*
 * Has fields:
 * + Round: int
 * + Floor: int
 * + Ceil: int
 * + Frac: real
 */

// Implement class StringValue
/*
 * Has fields:
 * + Split: function (string) -> []
 * + SplitWS: function () -> []
 * + Join: function ([]) -> string
 * + Lower: string
 * + Upper: string
 * + Length: int
 * + Slice: function (int, int, int) -> string  // start, stop, step
 */

// Implement class NoneValue

// Implement class BoolValue

// Implement class ArrayValue

// Implement class TupleValue

// Implement class FuncValue

// Implement class StringSplitFunction

// Implement class StringSplitWSFunction

// Implement class StringJoinFunction

// Implement class StringSliceFunction

}  // namespace runtime
