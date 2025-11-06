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
10. Chain comparison optimization
11. Fields in built-in objects
12. Zero division warning
13. Expression statement removal
14. Fixed types of expressions with unknown components (unary not, +, -; logical operators; comparisons)
15. Warnings for accessing a None
16. Variable existence checking (when declaring and referencing)
17. Tuple validation
18. Tuple concatenation
19. Array concatenation
20. Unused variables
21. Unused assigned values
22. Dead code after exit/return (with path checking)
23. 'if' branching removal if condition is known (condition removal if pure)
24. Short 'if' -> Normal 'if'
25. Check if 'while' condition is false for the first iteration
26. Check if 'while' condition is a bool for the first iteration
27. Variables used from within a loop are made unknown
28. Variables go out of scope
29. Ranged 'for': variable is int; checking if bounds are int
30. Foreach 'for': collection is [] or {}
31. Exit only in cycle
32. Return only in functions
33. Function return type and purity inference
34. Fields assignable only in tuples
35. Array items assignable only in arrays
36. Index evaluation for tuples
37. Field accesses evaluated and checked for known values
38. Subscript accesses evaluated and checked for known values
39. External variables have unknown types from inside a function body, until assigned
40. Value preview with recursion :)
