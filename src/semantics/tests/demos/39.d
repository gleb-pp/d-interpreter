// 39. For-variable can be redeclared inside the cycle body's scope

for i in 1..10 loop  // i is unused
    var i := 3   // ok: inner scope
    print i, "\n"
end
