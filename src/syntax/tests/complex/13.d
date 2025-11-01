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

var linetostr := func(list, size) is
    var spantostr := func(list, from, to) is
        if from = to then
            if list[from] is none then return "  "; end
            return list[from]
        end
        var m := (from + to) / 2
        return spantostr(list, from, m) + spantostr(list, m + 1, to)
    end
    return spantostr(list, 1, size)
end

print " 802.11"
var width := sqrt(2 * HEIGHT * HEIGHT) + 3
var center := width / 2 + 1
var grid := []
var i
for i in 1 .. HEIGHT loop grid[i] := []; end
var arc
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

var line
for line in grid loop
    print linetostr(line, width)
end
