//  32. Function return type and purity inference
var f := func () is
    var six := 6
    if six > 5 then
        return 3
    else
        return 4
    end
end

print f + 1
