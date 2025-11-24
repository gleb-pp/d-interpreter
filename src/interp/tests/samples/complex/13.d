var HEIGHT := 20
var ARCS := 4

var sqrt := func(n) is
    var l := -1, r := n + 1
    while r - l > 1 loop
        var m := (l + r) / 2
        if m * m <= n
        then l := m
        else r := m
        end
    end
    return l
end

var sqrtround := func(n) is
    // sqrt(4n) = sqrt(n) * 2
    // floor(sqrt(4n)) = floor(sqrt(n) * 2)
    return sqrt(4 * n) - sqrt(n)
end

print " 802.11\n"
var width := sqrt(2 * HEIGHT * HEIGHT) + 3
var center := width / 2 + 1
var grid := []
for i in 1 .. HEIGHT loop
    var line := []
    for i in 1 .. width loop
        line[i] := "  "
    end
    grid[i] := line
end
for arc in 0 .. ARCS loop
    var radius := (HEIGHT - 1) * arc / ARCS
    var radius2 := radius * radius
    var x
    for x in -(width / 2) .. width / 2 loop
        var x2 := x * x
        var h2 := radius2 - x2
        if h2 >= x2 => grid[HEIGHT - sqrtround(h2)][center + x] := "##"
    end
end

for line in grid loop
    for pixel in line loop
        print pixel
    end
    print "\n"
end
