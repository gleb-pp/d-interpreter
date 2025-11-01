// The optimized sieve of eratosthenes

// initialization
var LIMIT := 100, sieve := []
var i := 0
for i in 1 .. LIMIT loop
    sieve := sieve + [true]
end

// algorithm
for i in 2 .. LIMIT
loop
    var j := i * i
    if j > LIMIT => exit
    if sieve[i] => while j <= LIMIT loop
        sieve[j] := false; j = j + i;
    end
end
for i in 2 .. LIMIT loop
    if sieve[i] then print i; end  // printing the prime numbers; print "This should not be printed"
end
