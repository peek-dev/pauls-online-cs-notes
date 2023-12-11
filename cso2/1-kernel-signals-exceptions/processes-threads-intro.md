# Processes and Threads

A **process** is a unit of software in an operating systems context that parallels a running program. A process has the full suite of tools (stack, heap, globals and read-only, address translation, core(s) and thread(s), etc.) required to execute code on a computer. The process classification is all the operating system really needs to make, since differences between programs (e.g., a web browser and a terminal) can be simplified as "different processes having different needs". 

Processes essentially run in "virtual machines" with their own memory, processor cores, and more. Behind the scenes, the OS maintains data structures within kernel memory to map processes, their memory, and state to the code that will run.

## Threads

Each "processor core" (discrete computational unit) within a process is known as a **thread**, where each thread has an independent slate of program registers, a program counter, condition codes, and more state. Given this, threads can change their independently allocated state without concern for other programs' states (**thread safety**).

The operating system loads a thread's values and state into processor registers and appropriate condition codes.

## Virtual address spaces

Each process's "view of memory" is more formally a _virtual address space_ which is maintained independently of other processes' address spaces; like with individually allocated thread state, each process can modify memory within its address space at will without worry for other programs' states.

Virtual memory encompasses the tools which implement virtual address spaces, often through _address translation_ and the use of special kernel-maintained data structures like page tables.

## Context Switches

Changing the thread or process state loaded onto a processor core is a **context switch**. Major _time-sharing_ operating systems (Windows, MacOS, Linux) use hardware timers to create exceptions for _automated_ context switching regardless of how long it takes threads and processes to actually conclude their comptutation. 

Automated context switching also facilitates concurrency; quick and smooth context switching creates the illusion that more processes are running on the computer than it has space for, strictly speaking (with respect to physical limits on cores and threads).

