//  8. Xor reordering
var f := func () is
    print "f"
    return true
end // known that f returns bool

var val := f()

print true xor val xor true xor val xor false
// expect false xor val xor val
