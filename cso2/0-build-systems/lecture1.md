**Lecture 1: August 22, 2023**

# Themes

* Automating building software: libraries, incremental compilation, makefiles
* Sharing machines: multiple programs on one machine, multiple users, permissions, virtual memory, context switching, concurrency/parallelism, etc.
* Parallelism and concurrency: how to do "two things at once", whether actually or apparently
* Under the hood of sockets: networking layered design and implementing secured communication
* Under the hood of fast processors: caching, hidden parallelism, avoiding idle time

# make

-L. -lreadline : "_L_ook here and _l_ink against lib"readline".\*

# Virtual memory

Process A addresses: virtual --> OS mapping (page tables) --> Physical memory: Processes' code/data, etc.

# Concurrency and parallelism

* Interrupts
* Hardware timers and exceptions
* Multiple cores and threads: how to coordinate?
    * _Processor multiplexing_ to divide up at the individual core level

# Permissions

How do I make my directories _reasonably_ secure?

Why can I run some executables, but _not_ others?

# Under the hood of sockets

A socket is a file (thinking of a "talking to the terminal" abstraction). Implementing this involves engaging multiple layers of network protocols. Thinking of them as layers:
* Application layer (HTTP, HTTPS; SSH)
* Transport (TCP; UDP)
* Network (IP)
* Link (Ethernet and WiFi)
* Physical (the bits on wires)

How do we implement secure communication? How does a website we visit _really know_ it's us? How does it verify? Are these measures perfect?

# Under the hood of fast processors

* Caching: data and instructions, what gets used once gets used again, store on-chip vs. in memory, etc.

Performance examples:

```x86asm
example1:
    movq	$10000000000, %rax
loop1: 
    addq	%rbx, %rcx
    decq	%rax
    jge		loop1
```

```x86asm
example2:
    movq	$10000000000, %rax
loop2: 
    addq	%rbx, %rcx
    addq	%r8, %r9
    decq	%rax
    jge		loop2
```

# Logistics

Homeworks: due by 11:59pm on Fridays.

Labs: due by 11:59am the next day (Thursday morning for Wednesday labs).
_At least_ 50% credit for in-person check-off for reasonable work.

Lab collaboration is permitted! Just indicate who you worked with in/on your files.

## Homeworks

These are _individual_ and due on Friday. 

## Homework and Lab Automatic Testing

Some homeworks and labs are automatically graded with some delay after you submit.

If autograder encounters strange results, the autograder will "punt" the submission to a human grader.

# Warmup assignment

First homework:
* C function to split a string into array of strings with dynamic memory allocation.
* Main program will call function using input and command-line arguments
* Write a Makefile for the programs

# Quizzes

Open-book, open-note, (reasonably) open-internet with exception of collaborating with Stack Overflow or classmates.

Released evening after Thursday lecture and get them done **over the weekend!**

# Readings

DO THESE!

# Lecture/assignment relationship
Generally, quizzes will occur after lecture and lab coverage.
Labs will occur after lecture coverage
Homeworks will occur after lab coverage.

# Exams
* 1 final exam

Quizzes count for more; also, more homeworks and labs to account for this.

# Development environment

Official: Department machines.
Use x86-64 Linux!

# Office Hours

Office hours will be posted on a calendar on the website.

Piazza: use private questions for homework code and similar sensitive discussion.

# Collaboration
Labs: you can (_and probably should!_) work with other students

Homeworks are individual. You can look up conceptual questions, but don't ChatGPT the homework, ask about it on Stack Overflow, or generally plagiarize.

# Late policy

No late quizzes (they will be gone over in lecture). Late homeworks: 90% between 0 and 72 hours late. (BASED!)

Labs are due 11:59am Thursday mornings with 90% max credit available from 0-24 hours late.

# Excused lateness

"No-questions asked"

_Email Professor Reiss about SC_.

# Building

There are in fact _many_ files often involved in building a C program, even a simple one. 

Question: some man pages list sources as standard C library (`-lc`). Is `-lc` a default option for `gcc` and `clang`?

Question: can you convert a dynamic library to a static library or vice versa?

With dynamic linking, will the OS "reserve" the dynamic library in its code so that it can quickly switch out pages to various processes?

So, what's the benefit of static linking? Freezing the library "in amber" to statically link against has advantages in that the library will be have _determinstically_ for each time you invoke the program which links against it.

## File extensions

| name			| definition					|
| ---------------------	| ---------------------------------------------	|
| .c			| C source file 				|
| .h			| C header file 				|
| .s or .asm 		| Assembly file					|
| .o or .obj 		| Object file 					|
| (none) or .exe	| Executable 					|
| .a or .lib 		| Static library (collection of object files)	|
| .so, .dll, or .dylib 	| Dynamic library ("shared object")		|

-L (relative or absolute path to library fine)

# Shared libraries
Create: compile object files with `-fPIC` flag for **P**osition **I**ndependent **C**ode.

Then, build with: `cc -shared [...] -o libfoo.so`

Examples with `-fPIC`?

## Why can't I find my shared library?

`clang -o main main.c -L. -lexample`: this is where to look _when we build_, not really _when we run_.

How to fix this? We could put it in `/usr/lib/`, but we may not be able to. 

We could also modify our environment variables directly or with our compiler:
```sh
$ LD_LIBRARY_PATH=. ./main # this changes the environment ONLY for the shell which runs `main`
```

```sh
$ export LD_LIBRARY_PATH=.
```

With the compiler:
```sh
$ clang -o main main.c -L. -lexample -Wl,-rpath .
```

If you _statically link_, these extra steps aren't necessary.

