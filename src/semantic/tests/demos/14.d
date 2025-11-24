//  14. Expression statement removal
var s := "aba ca  ba"

s + s        // warning
s.SplitWS()  // warning
s := s.SplitWS()  // ok

print s
