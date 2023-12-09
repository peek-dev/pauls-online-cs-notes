# The `make` tool

The `make` utility reads a file (customarily called a "makefile" with the default name `Makefile`) and executes commands to update a project. When deciding how to update, `make` checks file modification times to avoid repetitive steps that aren't actually needed. Makefiles are great for streamlining build processes and avoiding time-consuming work required to memorize, type out, and manually invoke steps like compilation.

## Rules

The **rule** is the central component of a makefile, consisting of:
1. A **target**: the file which will be created or updated
2. A list of **dependencies** (also "prerequisites"): if the _target_ is missing or is older than one dependency, _system commands_ will be run
3. A list of **system commands** (also the "recipe"): shell commands intended to make or remake the _target_

The **target** must begin the line with no leading whitespace and end in a colon. Everything after that colon to the end of the line is a **dependency**. Single-tab-indented lines following the dependencies are the **system commmands**.

> Example 1: 
> The following will build `hello.o` if `hello.c` or `hello.h` is newer than `hello.o`. Otherwise, the rule will do nothing.
>
```
hello.o: hello.c hello.c
     clang -c hello.c
```
> 

Importantly, running `make` directs it to read the `Makefile` and execute the _first_ rule it finds unless you specify another rule manually by `make [rule]`.

If a dependency is the target of another rule, that other rule will execute first.

> Example 2:
```
runme: hello.o bye.o main.c main.h
    clang main.c hello.o bye.o -o runme

hello.o: hello.c hello.h
    clang -c hello.c

bye.o: bye.c bye.h
    clang -c bye.h
```
> Since `hello.o` and `bye.o` are _dependencies_ for the first rule where `runme` is the target, `make` will inspect the rule for which `runme` is the target and run the rules targeting `hello.o` and `bye.o` beforehand to ensure that the `runme` rule is completely fulfilled.

## Macros

Makefiles use macros (variables) to distinguish files from rules and to allow easy flexibility in compilers, flags, and more.

Variables are defined with `NAME := meaning` with names traditionally in all caps. To use a variable, enclose the name in parentheses and prefix a `$`: `$(NAME)`.

Here is a table of common variables in makefiles:

| Name	| Example	| Notes	|
| ---	| ---		| ---	|
| CC	| `clang`	| The **C** **c**ompiler to use. |
| CFLAGS | `-O2 -g`	| Compile-time flags for the C compiler. |
| LDFLAGS | `-static`	| Link-time flags for the compiler, placed before the list of object files.|
| LIBS/LDLIBS | `-lm`	| Link-time flags indicating the libraries for the compiler to link against, placed after a list of object files. |
| CXX | `clang++` 	| The **C++** **c**ompiler to use. |
| CXXFLAGS | `-O2 -g`	| Compile-time flags for the C++ compiler |

Defining `LDFLAGS :=` and/or `LIBS :=`, then referencing them for all relevant rules, is helpful even if you leave them blank. In this way, if you need to link against another library later, you can update the definition in the header before your rules instead of having to add detail for each rule.

## Targets that aren't files

## Pattern rules

# Multi-file project design

## Glossary

## Common patterns 

### Creating a shared library
1. Compile each `.c` file into a `.o` file using the `-fPIC` compiler flag (directive for "position-independent code") so the code can be shared by multiple applications.

2. Invoke `ld` to combine `.o` files using a `gcc` or `clang` frontend wrapper like the following:
```sh
clang -shared \
    all.o your.o object.o files.o \
    -o libyourname.so
```

### Creating a static library
1. Compile each `.c` file into a `.o` file

2. Use `ar` to collect `.o` files into a static library `.a` file.

3. _Preferred_: use `make` syntax to make a static library:
    1. Use the library name as the target
    2. Use the library name followed by an object file for dependencies
    3. Use the `ranlib` command as the system command

Example:
```sh
libname.a: libname.a(all.o) libname.a(your.o) libname.a(object.o) libname.a(files.o)
    ranlib libname.a
```
