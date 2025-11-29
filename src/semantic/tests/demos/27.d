//  27. Variables go out of scope
var unk

var unk_trigger := func () is print; end
unk_trigger()

// ^ that code makes 'unk' have an unknown type ^

var i := 1

if unk then  // this is not optimized because unk is unknown
    var j := 2
end

print j
