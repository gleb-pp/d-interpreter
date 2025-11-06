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
19. Tuple concatenation
20. Array concatenation
21. Unused variables
22. Unused assigned values
23. Dead code after exit/return (with path checking)
24. 'if' branching removal if condition is known (condition removal if pure)
25. Short 'if' -> Normal 'if'
26. Check if 'while' condition is false for the first iteration
27. Check if 'while' condition is a bool for the first iteration
28. Variables used from within a loop are made unknown
29. Variables go out of scope
30. Ranged 'for': variable is int; checking if bounds are int
31. Foreach 'for': collection is [] or {}
32. Exit only in cycle
33. Return only in functions
34. Function return type and purity inference
35. Fields assignable only in tuples
36. Array items assignable only in arrays
37. Index evaluation for tuples
38. Field accesses evaluated and checked for known values
39. Subscript accesses evaluated and checked for known values
40. External variables have unknown types from inside a function body, until assigned
41. Value preview with recursion :)
