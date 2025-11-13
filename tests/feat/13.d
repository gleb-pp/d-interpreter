var input := func () => 10

var x := input()

var z := 0

print x / z  // warning: x is unknown - can be int
print 1.3 / z  // ok, precomputed NaN

var fl
if x = 10 then
    fl := 1.3
else
    fl := 4.5
end
print fl / z  // ok, fl is a real, this will be NaN
