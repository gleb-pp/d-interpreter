var arr := ["a", "b", "c"]
arr[10] := "d"
arr.Del(2)
print arr, "\n" // [ [1] a, [3] c, [10] d ]
var brr := ["a"]
brr[3] := "c"
brr[10] := "d"
print arr = brr, "\n"  // false: pointers are different

print arr.Indices, "\n" // [ [1] 1, [2] 3, [3] 10 ]
