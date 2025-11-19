// fibonacci

var seq := [1, 1], i
for i in 3 .. 100 loop
    seq := seq + [seq[i - 1] + seq[i - 2]]
end
for i in seq loop print i, "\n"; end
