# Runtime

This library defines the runtime objects, and their types, and how they interact.

Types defined here:

- `Type` is the base class for all runtime types:
    - `IntegerType`;
    - `RealType`;
    - `StringType`;
    - `NoneType`;
    - `BoolType`;
    - `ArrayType`;
    - `TupleType`;
    - `FuncType` contains the information about parameter types, return type, and possible side effects of a function
    (purity);
    - `UnknownType` is a placeholder type used in semantic checking when the type of a value is not determined;
- `RuntimeValue` is the base class for all runtime values:
    - `IntegerValue`;
    - `RealValue`;
    - `StringValue`;
    - `NoneValue`;
    - `BoolValue`;
    - `ArrayValue`;
    - `TupleValue`;
    - `FuncValue` is the base class for all **built-in** functions (user-defined functions have to be called differently
      and are defined in the `interp` library):
        - `ArrayDelFunction` is a method of arrays, it accepts an integer as the index and deletes the element or throws
          if the element is not found;
        - `StringSplitFunction` is a (pure) method of strings, it accepts a separator string and returns an array of
        substrings of the original;
        - `StringSplitWSFunction` is a (pure) method of strings, it accepts no arguments and returns an array of
        substrings that were separated by whitespace in the original;
        - `StringJoinFunction` is a method of strings, it accepts an array of strings and returns a string that is a
        concatenation of all strings in the array, separated by the self-string;
        - `StringSliceFunction` is a method of strings, it accepts 3 integers (start, stop, step) and returns a
        subsequence of characters (as a string) that starts at the *start* index, stops **before** the *stop* index
        (*stop* is exclusive), and such that the difference in consecutive indices is *step*. *Step* cannot be 0.
- `DRuntimeError` is an error message that is *returned*, not thrown, from functions that are not successfully
performed.
- `RuntimeValueResult` alias type is usually returned from methods of the above classes. It is a union of:
    - a shared pointer to a `RuntimeValue` if an operation is successful;
    - nothing if an operation is not supported in some way;
    - `DRuntimeError` if there is an exceptional situation;
- `TypeOrValue` is a union type of `Type` and `RuntimeValue` that is used in semantic analysis.

## Operator results in types

### Notation

| Notation | Meaning       |
| -------- | ------------- |
| int      | `IntegerType` |
| real     | `RealType`    |
| str      | `StringType`  |
| none     | `NoneType`    |
| bool     | `BoolType`    |
| []       | `ArrayType`   |
| {}       | `TupleType`   |
| func     | `FuncType`    |
| ?        | `UnknownType` |
|          | **error**     |

### Binary Plus

| **+** | int  | real | str | none | bool | [] | {} | func | ?    |
|-------|------|------|-----|------|------|----|----|------|------|
| int   | int  | real |     |      |      |    |    |      | ?    |
| real  | real | real |     |      |      |    |    |      | real |
| str   |      |      | str |      |      |    |    |      | str  |
| none  |      |      |     |      |      |    |    |      |      |
| bool  |      |      |     |      |      |    |    |      |      |
| []    |      |      |     |      |      | [] |    |      | []   |
| {}    |      |      |     |      |      |    | {} |      | {}   |
| func  |      |      |     |      |      |    |    |      |      |
| ?     | ?    | real | str |      |      | [] | {} |      | ?    |

### Binary Minus, Multiplication, and Division

| **-, *, /** | int  | real | str | none | bool | [] | {} | func | ?    |
|-------------|------|------|-----|------|------|----|----|------|------|
| int         | int  | real |     |      |      |    |    |      | ?    |
| real        | real | real |     |      |      |    |    |      | real |
| str         |      |      |     |      |      |    |    |      |      |
| none        |      |      |     |      |      |    |    |      |      |
| bool        |      |      |     |      |      |    |    |      |      |
| []          |      |      |     |      |      |    |    |      |      |
| {}          |      |      |     |      |      |    |    |      |      |
| func        |      |      |     |      |      |    |    |      |      |
| ?           | ?    | real |     |      |      |    |    |      | ?    |

Remark: Division is pure iff it is known that it will be real division because integer division may throw.

### Binary (In)Equality

| **=, /=** | int  | real | str  | none | bool | []   | {} | func | ?    |
|-----------|------|------|------|------|------|------|----|------|------|
| int       | bool | bool |      |      |      |      |    |      | bool |
| real      | bool | bool |      |      |      |      |    |      | bool |
| str       |      |      | bool |      |      |      |    |      | bool |
| none      |      |      |      |      |      |      |    |      |      |
| bool      |      |      |      |      |      |      |    |      |      |
| []        |      |      |      |      |      |      |    |      | bool |
| {}        |      |      |      |      |      |      |    |      |      |
| func      |      |      |      |      |      |      |    |      |      |
| ?         | bool | bool | bool |      |      | bool |    |      | bool |

### Binary Ordering

| **<=>** | int  | real | str  | none | bool | [] | {} | func | ?    |
|---------|------|------|------|------|------|----|----|------|------|
| int     | bool | bool |      |      |      |    |    |      | bool |
| real    | bool | bool |      |      |      |    |    |      | bool |
| str     |      |      | bool |      |      |    |    |      | bool |
| none    |      |      |      |      |      |    |    |      |      |
| bool    |      |      |      |      |      |    |    |      |      |
| []      |      |      |      |      |      |    |    |      |      |
| {}      |      |      |      |      |      |    |    |      |      |
| func    |      |      |      |      |      |    |    |      |      |
| ?       | bool | bool | bool |      |      |    |    |      | bool |

### Unary Plus, Minus

| Operand | Result |
|---------|--------|
| int     | int    |
| real    | real   |
| str     |        |
| none    |        |
| bool    |        |
| []      |        |
| {}      |        |
| func    |        |
| ?       | ?      |

### Unary Not

It only works on `UnknownType` and `BoolType` and produces a `BoolType`.

### Subscript (square brackets)

Subscript only accepts an integer argument inside the brackets and is only defined for arrays. It always returns an
`UnknownType` during semantic analysis because array values are never precomputed. For `ArrayValue`s, it properly
returns the stored value or a `DRuntimeError` if the provided index is not in the array.

## Built-in fields and methods

Fields described here are fictional, they cannot be assigned to.

### int

- `Round` returns a clone of the integer;
- `Floor` returns a clone of the integer;
- `Ceil` returns a clone of the integer;
- `Frac` returns a real value `0.0`.

### real

- `Round` returns an integer that is obtained by rounding the real value to the closest whole number;
- `Floor` returns the greatest integer less than or equal to the real value;
- `Ceil` returns the smallest integer greater than or equal to the real value;
- `Frac` returns the fractional part of the real number, preserving the sign.

`Round`, `Floor`, and `Ceil` return 0 if the real value is not a valid real number (e.g. an infinity or Not a Number
(NaN)).

### string

- `Split(sep: str) -> []` is a `StringSplitFunction`;
- `SplitWS() -> []` is a `StringSplitWSFunction`;
- `Join(strings: []) -> str` is a `StringJoinFunction`;
- `Lower` returns this string, but in lowercase;
- `Upper` returns this string, but in uppercase;
- `Length` returns the number of bytes in the UTF-8 representation of the string;
- `Slice(start: int, stop: int, step: int) -> str` is a `StringSliceFunction`.

### Array `[]`

- `Del(index: int) -> none` is an `ArrayDelFunction`;
- `Indices` returns an array of integers that are the indices of this array in an increasing order;
- `Length` returns the number of elements in the array.

## Known issues

### Strings are encoded as UTF-8

Non-ascii characters are treated as several separate characters.

### Memory leaks

If an object references itself (e.g. a list contains itself), the object will never be destroyed. This behaviour is
difficult to prevent, so it is left unfixed.

### Comparison of collections compares elements by pointers

Right now, two arrays are equal if they contain exactly the same objects in exactly the same indices. Intuitively, the
object should not be the same, they should be equal. But this introduces problems, for example, in the case where two
lists are elements of each other. In this case, comparison by value would result in an infinite recursion.

The following python code, for example:
```python
a = []
b = []
a.append(b)
b.append(a)
print(a == b)
```
Produces:
```text
Traceback (most recent call last):
  File ..., line 5, in <module>
    print(a == b)
          ^^^^^^
RecursionError: maximum recursion depth exceeded in comparison
```

### No tests

The `runtime` library relies on tests of `semantics` and `interp` libraries.
