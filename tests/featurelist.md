1. Value & type precomputation, typechecking
2. Precomputation exceptions
3. Built-in function argument checking & precomputation
4. Argument count checking for known functions
5. Closure parsing
6. Expression optimization
7. Lazy and/or checking, code deletion
8. Xor reordering
9. Numeric sum reordering
10. Sequential concatenation optimized
11. Chain comparison optimization
12. Fields in built-in objects
13. Zero division warning
14. Expression statement removal
15. Fixed types of expressions with unknown components (unary not, +, -; logical operators; comparisons)
16. Warnings for accessing a None
17. Variable existence checking (when declaring and referencing)
18. Tuple validation
19. Unused variables
20. Unused assigned values
21. Dead code after exit/return (with path checking)
22. 'if' branching removal if condition is known (condition removal if pure)
23. Short 'if' -> Normal 'if'
24. Check if 'while' condition is false for the first iteration
25. Check if 'while' condition is a bool for the first iteration
26. Variables used from within a loop are made unknown
27. Variables go out of scope
28. Ranged 'for': variable is int; checking if bounds are int
29. Foreach 'for': collection is [] or {}
30. Exit only in cycle
31. Return only in functions
32. Function return type and purity inference
33. Fields assignable only in tuples
34. Array items assignable only in arrays
35. Index evaluation for tuples
36. External variables have unknown types from inside a function body, until assigned
37. Value preview with recursion :)   // runtime
38. Array concat, tuple concat  // runtime
