//  34. Array items assignable only in arrays
var unknown := 1
func () is if unknown = 1 then unknown := 9; else unknown := []; end; end ()
// the checker does not know what unknown is at this point

var tuple := {first := 0, second := 1}, arr := [5, 6]

arr[3] := 1  // ok
unknown[-100] := 3 // ok
tuple[1] := 5 // error
