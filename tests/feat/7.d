var f := func () is
    print "f"
    return true
end // known that f returns bool

print 4 = 5 or 5 = 6 or f()

// this is optimized to 'Print{Or{f()}}'. I do not remove the Or with one operand because Or also checks that f() is a
// bool.

print 5 = 5 and 5 = 6 and f()
