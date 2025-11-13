//  22. 'if' branching removal if condition is known (condition removal if pure)
var a := 1, b := 3
if a = 1 or
        b = 2 then
    print "ok"
else
    print "nok"
end
