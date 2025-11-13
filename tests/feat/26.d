var i := 1

for 0..10 loop
    if i is int then
        i := "1"
    else
        i := 1
    end
end

print i is int  // type of i is unknown after the loop, this is not simplified

i := 1

for 0..10 loop
        print "ok"
end

print i = 1  // this is known to be true because i is not reassigned in the loop
