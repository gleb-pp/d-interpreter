#include "runtime/types.h"
using namespace std;

/*
 * In subclasses of `Type`, returning {} where the return type is std::optional, means "operation not supported".
 * Returning UnknownType means that the operation could be supported.
 * If the virtual method is not overridden, the default is to return {} ("not supported").
 */

namespace runtime {

//Implement class Type

//Implement class IntegerType
/*
 * Has fields:
 * + Round: int = this
 * + Floor: int = this
 * + Ceil: int = this
 * + Frac: real = 0.0
 */

//Implement class RealType
/*
 * Has fields:
 * + Round: int
 * + Floor: int
 * + Ceil: int
 * + Frac: real
 */

//Implement class StringType
/*
 * Has fields:
 * + Split: function (string) -> []
 * + SplitWS: function () -> []
 * + Join: function ([]) -> string
 * + Lower: string
 * + Upper: string
 * + Slice: function (int, int, int) -> string  // start, stop, step
 * + Length: int
 */

//Implement class NoneType

//Implement class BoolType

//Implement class ArrayType

//Implement class TupleType

//Implement class FuncType

//Implement class UnknownType

}  // namespace runtime
