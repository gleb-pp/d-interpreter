# The **D** Language 🔥🔥🔥

An interpreter for the [dynamic
language](https://github.com/Error10556/d-interpreter/blob/main/proj-description/Project%20D.pdf) implemented in С++.

# Brief Language Description

The **D** language is dynamically-typed and similar to python in some ways. In short,

- a program is a sequence of statements separated by newlines or semicolons;
- a statement can:
    - declare variables: `var i := 1, s := "some text..."`,
    - print something: `print "Hello, world!\n"`,
    - assign something: `person.username := "coolguy"`,
    - introduce branching: `if a < b then print a; else print b; end`, `if a < b => a := b`
    - be a `while` loop: `while i < 10 loop print i, "\n"; i := i + 1; end`,
    - be a `for` loop: `for i in 1..10 loop sum := sum + i; end`,
    - be an endless loop: `loop print "y\n"; end`,
    - exit from a loop (literally `exit`),
    - return something: `return 42`,
    - or be any expression: `my_function(43 + 3)`;
- the only object types are `int` (an arbitrarily-huge integer), `string`, `real` (`long double` in C++), `bool`,
`none`, `[]` (arrays), `{}` (tuples), and `func`;
- the language supports operators `*`, `/` (integer division if both operands are integers, otherwise this is
floating-point division), `+` and `-` (both unary and binary), `<`, `<=`, `>`, `>=`, `=` (equals), `/=` (not equals),
`and`, `or`, `xor`, `not`, and the typechecking operator `is`;
- to define a function, assign a function literal to a variable:
```
var max := func (a, b) is
    if a < b then
        return b;
    else
        return a;
    end
end
```
- to define a recursive function, assign its literal to an already declared variable:
```text
var factorial
factorial := func (n) is
    if n < 2 => return 1
    return n * factorial(n - 1)
end
```
- indices usually start from 1;
- arrays' indices do not have to be consecutive, you can insert values into an array by assigning to empty indices:
```text
var arr := ["a", "b", "c"]
print arr[1], "\n"  // a
arr[10] := "d"
print arr, "\n"  // [ [1] a, [2] b, [3] c, [10] d ]
arr := arr + arr
print arr, "\n"  // [ [1] a, [2] b, [3] c, [10] d, [11] a, [12] b, [13] c, [20] d ]
```
- tuples have indexing, and their elements can be named:
```text
var t := {  // yes, line breaks are allowed in some places
    username := "coolguy",
    password := "123",
    80
}
print t.username, "\n"  // coolguy
print t.1, "\n"  // coolguy
print t.3, "\n"  // 80
```
- some objects have fields and methods! For example, `int`s and `real`s have rounding, `string`s have `Length`,
`Split(sep)`, `Join(array)`, `Slice(start, stop, step)`. Access them with a period: `s.Split(";")`.

# Building & Installation

`cmake` and a C++20 compiler are required. Only Linux systems are supported.

The project provides 2 things:
1. an interpreter for the **D** language, `dinterp`;
2. a statically linked library with C++ headers, `libdinterptools.a`, provided as a CMake package.

- To install both, run `scripts/install_all`.
- To install only the interpreter, run `scripts/install_dinterp`.
- To install only the library, run `scripts/install_sdk`.
- To not install anything and simply build the binaries, run `scripts/build` and check the `build` folder.

Run `dinterp -h` for usage instructions.

If you have installed `dinterp`, it is possible to create scripts with shebangs:
```bash
cat >program.d <<EOF
#!/bin/env dinterp
print "Hello, world!\n"
EOF

chmod +x program.d
./program.d
```

### Uninstallation

After installation, CMake creates a file `build/install_manifest.txt`; this file contains all installed file locations.
Remove these files manually or with a simple loop:
```bash
cd build
while read file; do
    sudo rm "$file"
done <install_manifest.txt
# Unfortunately, this does not remove the left-over empty directories.
```

If you did not change the default install locations, remove these to completely wipe the installation:
```bash
sudo rm /usr/local/bin/dinterp  # if the interpreter has been installed
# if the library has been installed:
sudo rm /usr/local/lib/libdinterptools.a  # the library
sudo rm -r /usr/local/lib/cmake/dinterp   # the package information
sudo rm -r /usr/local/include/dinterp     # the headers
```

# Testing

## Full testing

`docker` is required, nothing else.

Run `scripts/test` to test the entire project.

Testing is done from within a docker container to test that installation works properly.

## Code testing

`cmake`, a C++20 compiler, and `GTest` ([repo](https://github.com/google/googletest)) are required.

Install `GTest` with this:
```bash
git clone https://github.com/google/googletest.git --depth 1 --branch main gtest
cd gtest
mkdir build && cd build
cmake ..
cmake --build .
sudo cmake --install .
cd ../.. && rm -r gtest  # optionally erase the source code
```

Run `scripts/test_noinstall`.

## CMake produces a cryptic error

If you intend to run `test_noinstall` after `test` or the other way around, remove the `/testbuild` directory first.

# Using the library

First, install the library with `scripts/install_sdk` or `scripts/install_all`.

See `scripts/docker-test/sample_user_project` for a bare-minimum example of a CMake project.

To compile without CMake, simply add `-ldinterptools` (and perhaps `-std=c++20`) compiler parameters. The
`-ldinterptools` parameter must appear **after** the source files!

# Implementation details

See `src/README.md`.

# Credits

- **Eugene Zouev**, an Innopolis University professor, for inventing the **D** toy language.
