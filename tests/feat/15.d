//  15. Fixed types of expressions with unknown components (unary not, +, -; logical operators; comparisons)
var a := "str"

var mutate := func () is
    print
end  // non-pure function: call resets all known values

mutate()

if (not a) is bool then
    print "true"
else
    print "false"
end
// will turn into:
// (not a) is bool  // because operations on unknowns are not pure; this preserves the error for runtime
// print "true"
