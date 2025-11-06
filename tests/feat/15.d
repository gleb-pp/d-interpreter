var a := 0  // will become unknown after the function call

var f := func (x) is
    if x > 0 => print "hello";
end,
    b := f(5)

print a + b
