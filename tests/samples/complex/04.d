// decorators

var printfunc := func(value) is print value; end
printfunc("normal\n")
// expected output:
// normal

var decor := func(f) is
    return func(value) is
        print "===-----===\n"
        f(value)
        print "===-----===\n"
    end
end

printfunc := decor(printfunc)
printfunc("decorated\n")
// expected output:
// ===-----===
// decorated
// ===-----===

var leavecontact := decor(func(_) is print "Call: 8 800 123 45 67\n"; end)
leavecontact(none)
// expected output:
// ===-----===
// Call: 8 800 123 45 67
// ===-----===
