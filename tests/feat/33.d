//  33. Fields assignable only in tuples
var unknown := 1
func () is if unknown = 1 then unknown := 9; else unknown := []; end; end ()
// the checker does not know what unknown is at this point

var tuple := {first := 0, second := 1}, nontuple := [5, 6]

tuple.first := 5  // ok
unknown.third := 3 // ok
nontuple.second := 1  // error
