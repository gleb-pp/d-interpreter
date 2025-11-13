//  21. Dead code after exit/return (with path checking)
var f := func () is
    var unk  // from within the loop, the type is not known

    for 0 .. 10 loop
        if unk then
            print "hello"
            print "world"
            return
            print "this is deleted"
            print "this is deleted"
        else
            exit
            print "this is also deleted"
        end
        print "this is deleted because we have (at least) exited in all branches"
    end
    print "this is kept because we could have NOT returned"
end

f()
