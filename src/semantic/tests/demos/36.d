//  36. External variables have unknown types from inside a function body
var a := 3

func () is
    if a is int => print "we do not know"
end ()
