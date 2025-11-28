# BigInt

This library only provides two classes:

- `BigInt` itself, and
- `ZeroDivisionException` runtime error.

In retrospective, adding the exception class was not a wise decision since it is not used anywhere in the interpreter;
an `std::runtime_error` would be sufficient.

`BigInt` provides many methods and operators, the semantics of which should be self-explanatory.

Algorithmic complexities of arithmetics (where both numbers are assumed to have lengths of $n$ 32-bit chunks):

| Operation        | Complexity      |
| ---------------- | --------------- |
| Addition         | $O(n)$          |
| Subtraction      | $O(n)$          |
| Multiplication   | $O(n\sqrt{n})$  |
| Division, Modulo | $O(n^2\log C)$  |
| Negation         | $O(1)$ in-place |

Here, $C$ is the maximum chunk value, which is a constant $2^{32}$.

$O(n\sqrt{n})$ multiplication is achieved with Karatsuba's algorithm.
