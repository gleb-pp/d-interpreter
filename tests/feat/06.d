var f := func () => 4;

var a := f() // known that a is an 'int'

var googol := 10000, temp := 10000  // 100 = 64 + 32 + 4
temp := temp * temp  // 8
temp := temp * temp  // 16
temp := temp * temp  // 32
googol := googol * temp // 32 + 4
temp := temp * temp  // 64
googol := googol * temp // 64 + 32 + 4

print 2 + a + 3 + googol
