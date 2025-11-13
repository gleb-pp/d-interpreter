//  23. Short 'if' -> Normal 'if'
var unk
func () is print; end ()

if unk => print 8 // ShortIfStatement is replaced with an IfStatement
// to see it, run dinterp -s 25.d, and then dinterp 25.d
