// decorators

var printfunc := func(value) is print value; end
printfunc("normal")
// expected output:
// normal

var decor := func(f) is
    return func(value) is
        print "===-----==="
        f(value)
        print "===-----==="
    end
end

printfunc := decor(printfunc)
printfunc("decorated")
// expected output:
// ===-----===
// decorated
// ===-----===

var leavecontact := decor(func is print "Call: 8 800 123 45 67"; end)
leavecontact()
// expected output:
// ===-----===
// Call: 8 800 123 45 67
// ===-----===
