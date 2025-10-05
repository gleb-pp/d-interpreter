// long arithmetic: 100!
var N := 100

var res := 1, i
for i in 1 .. N
loop res := res * i
end
print
print N, "! = ", res
print
