//  20. Unused assigned values
var unk
func () is print; end ()  // makes the checker forget that unk is a <none>

var _ := 80

var j

if unk then
    j := 1
    j := 2
else
    j := 3
end

print
