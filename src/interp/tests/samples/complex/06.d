// Church numerals

// 3 = ls. lz. s (s (s z))
// 2 = ls. lz. s (s z)
// mul = la. lb. ls. lz. a (b s) z
var append := func (list) => list + ["item"]
var two := func(s) is return func(z) is return s(s(z)); end; end
var three := func(s) is return func(z) is return s(s(s(z))); end; end
var mul := func(a) => func(b) => func(s) => func(z) => a(b(s))(z)
var six := mul(two)(three)
print six(append)(["initial","list"])
// expected output:
// ["initial", "list", "item", "item", "item", "item", "item", "item"]
