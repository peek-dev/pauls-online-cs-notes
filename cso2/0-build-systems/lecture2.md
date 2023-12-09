**Lecture 2: August 24, 2023**

# Last Time: Course Overview, Building, and Compilation

Building: compiling, assembling, linking, etc.

Source to object: compiling and assembling
    * Reads C file(s) and all #included files

Compiling objects into executable: reads object files and system files (linking)

-Lpath -lname: Identifying and locating libraries (static .a/dynamic .so)

Runtime search paths: -Wl,rpath PATH

Just because you _loaded_ the executable correctly doesn't mean it will run. Specifically, you'll need to be attentive to runtime search paths so that the executable can find the files it needs to source, particularly for shared libraries.

Recordings will be posted in the evening (processing needs to happen first, and this can take a while).

# Anonymous Feedback

McLeod Hall

C refresher? That's the _warmup_ assignment.

## C Exercise

```c
int array[4] = {10,20,30,40};
int *p;
p = &array[0]; // p = array;
p += 2;	// adds 2 * sizeof(int) to raw address; p = array[2];
p[1] += 1; // equivalent to *(p+1) += 1;
```

In actuality, you _can_ index off of a pointer (this is ok! Pointers and arrays are quite similar, so it works). Thus, `array` points to the following after the example code ones: `{10, 20, 30, 41}`.

## Avenues for Review
* Labs 9-12 from Spring 2023 CSO1

Past exercises:
* Implement strsep
* Dynamic array to linked list conversion

# Quiz demo

Everybody gangsta until the server for the quizzes is down??

If there are severe technical issues, the first quiz will be dropped (and this won't count against the "you get one dropped quiz").

# make

Unix build system which operates on _Makefiles_ and performs tasks based on their _rules_.

For each rule, if mtime of any prerequisite is later than the target's mtime, `make` reruns system commands (_recipe_) for that rule. Make _recursively_ applies these rules to make sure prerequisites are likewise up to date.

## Rule chains in `make`
```sh
program: main.o extra.o
    clang -o program main.o extra.o

extra.o: extra.c exrtra.c
    clang -c extra.c

main.o: main.c main.h extra.h
    clang -c main.c
```
Running `make` (assuming `program` is the first rule in the file) or `make program` (for explicit syntax) will check the prerequisites for _program_ recursively, likewise making sure that those prerequisites are up to date since they are the targets of other rules.

## `make` exercise:
```sh
W: X Y
    buildW
X: Q
    buildX
Y: X Z
    buildY
```

* W modified 1 minute ago
* X modified 3 hours ago
* Y DNE
* Z modified 1 hour ago
* Q modified 2 hours ago

Make W:
* Checks prereqs. Y DNE, so run Y:
    * Before buildY, checks prereqs. X older than Q, so
	* buildX
    * buildY
* buildW

## Variables and Macros


