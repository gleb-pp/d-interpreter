//scope
var i
for i in 0 .. 99 loop
    var j
    for j in 0 .. i loop end
end
print j // error: undefined variable 'j'
