**Lecture 4: August 31, 2023**

# Last Time

* Kernel mode: specialized software (the kernel) intercepts and performs potentially dangerous tasks on our behalf
    * Only OS code and routines (interrupt handlers, etc.) within restricted kernel memory are allowed to run in kernel mode.
* Exceptions: hardware calls certain routines in kernel mode upon detecting certain events.
    * Hardware may not initially know what to do, and hands off control to the OS. OS figures out the problem and passes off a next step to the programs and hardware
* System calls: intentionally triggered exceptions ("traps")
    * Deliberate asks to do something that requires kernel mode (hardware interactions, writes to the screen, reads from input, etc.)
* Other exceptions: general events where hardware needs OS help
    * Faulty instructions and similar errors: dividing by zero, invalid memory accesses, etc. ("faults")
    * I/O events: key and button presses, network packet reception, etc. ("interrupts")
    * Timer-generated exceptions to guarantee _time multiplexing_: the ability for multiple processes to get "their turn" to perform computations on a processor. ("timer exceptions")
    * Other unexpected or extraordinary events ("aborts")

External exceptions? --> (how to stop an infinite loop?)
    * I/O events (like pressing Ctrl+C to send SIGINT) which trigger an exception handler
    * OS-configured timer: OS sets this up and can generate exceptions based on hitting certain conditions (zero time, etc.). These timers don't know (and don't care) about what processes are running, etc.--its only job is to count down and notify the OS of an exception when a certain condition is met/exceeded
	* How does the timer know how long to wait? OS configures this at boot
	* This timer is _dynamically_ configurable within a processor (think of hardware timers--you can set those up on a microcontroller after power cycling!

# More on Last Time
* Address translation and address spaces
    * Address space provides program a "view" of memory with _virtual_ addresses which the OS _maps_ to real (_physical_) addresses
    * Mapping provides process separation in memory
    * OS chooses these mappings on actual hardware as needed
    * 1:1 -- one process:one address space; one mapping:one running program
* Time multiplexing
    * OS can perform _context switches_ to swap out which program (with associated state) is performing computation on the processor at once.

# The System Call Process

1. User mode: program encodes OS request (sets up `syscall` instruction) in registers, then executes `syscall`
2. Kernel mode: system call handler starts
    1. Handler reads registers to find out what program wants from the kernel
    2. Handler may do as requested or maybe not
3. System call handler returns and jumps back to user mode regardless of what happened

# Exception Process Generally
1. User mode: exception (synchronous/asynchronous)
2. Kernel mode: exception handler starts running
    1. OS handler handles whatever happened
    2. Exit exception handler    
3. Go back to running program code, which may be a different program (_special exit instruction_ to simulatneously change privilege level and jump back to user code)

#  Crash timeline
1. User mode: unsuccessful instruction happens
2. Kernel mode: exception handler starts running
    1. Handler _may or may not_ 

# Threads
A _thread_ is the illusion of a dedicated processor that gets provided to a process (one or more per process).

Threads have individually provisioned _state_:
    * Own program counter
    * Own registers
    * Own stack

# Switching programs
1. OS starts running somehow (an exception)
2. OS saves old registers and program counter
    1. Maybe some optimization possible in discarding context on abort
3. OS sets new registers (resets address mapping, perhaps) and jumps to new program counter

## Contexts
e.g., A running
* In CPU: A's context: registers, program counter, flags (SF, ZF), etc.
* In memory: Process A memory: code, stack, etc.
* In memory: Process B memory: code, stack, etc. (OS keeps track -- saves PTBR with context?)
    * OS memory: Process B _context_: registers, program counter, etc.

_Context switch_: OS swaps B context to CPU and A context to OS memory.

a. Program calls a function in std lib: neither (_it depends_ on the standard library function, but no in many cases).
    * _Calling_ a function does not trigger an exception. Whatever that standard library function may do (whatever it might _wrap_ in its role as a _system call wrapper_), though, could trigger an exception.
	* e.g., calling `printf()` itself doesn't trigger an exception, but `printf()` wrapping something like `write()` (which is itself a system call wrapper for )
b. Program writes a file to disk: _exception_, but not necessarily _context switch_.
c. Program A sleeps and lets B run: maybe _exception_ (is `sleep` a system call?), and definitely _context switch_.
d. Program exits -- context switch (_system call_) 
e. Program returns from one function to another -- probably neither (can under certain circumstances)
f. Program pops a value from stack -- probably neither (can be under certain circumstances by hitting page boundaries? -- not really. _Pushing_ might cross page boundaries, though, and trigger a page fault)
    * Stack deallocation _cannot_ happen automatically. 

Note: any context switch between different processes **must** have an exception preceding it.

# Exception Terminology
Names for exceptions _aren't standardized!_

Our readings' terminology:
* Interrupts: externally-triggered asynchronous events
* Faults: events/errors in program
* Traps: intentionally-triggered events (system calls, etc.)

# Processes
**Process** = _thread(s)_ + _address space_

A process is a whole complement of tools required to simulate a _dedicate machine_:
* Thread: "own CPU"
* Address space: "own memory"

# Signals
Unix-like OS features which are process-readable exceptions (where exceptions are meant to be hardware-generated and OS-readable)

* Can be triggered by external process (`kill` or system call)
* Can be triggered by special events (Ctrl+C) or other events:
    * Segmentation fault
    * Illegal instruction
    * Divide by zero

* _Signal handler_ can be invoked like an exception handler, though _signal handlers_ must be registered or installed _per process_ instead of _per OS_ as with exception handlers.

## Exceptions vs. Signals

| (Hardware) exceptions				| Signals	|
| ---------------------------------------------	| ------------- |
| Handler runs in kernel mode			| Handler runs in user mode |
| Hardware decides when				| OS decides when (hardware "forwards" exception for user-mode signal handler to run) |
| Hardware needs to save program counter	| OS saves PC and registers |
| Processor's next instruction changes 		| Thread's next instruction changes |

## Signal example

In a normal program, the default action for many signals is to terminate. 

e.g., for a normal program, Ctrl+C will send SIGINT to process and terminate. The process can register or install a _signal handler_, however, that runs a specialized dedicated routine to deal with the signal.

`sigaction(SIGINT, &act, NULL);` -- one call per signal you wish to handle (`act` is a `struct sigaction` which includes a handler pointer and is not itself a function)
`struct sigaction` -- include pointer to handler

Where are `sigaction` and `struct sigaction` defined? `<signal.h>`.

## Linux signals from `<signal.h>`

SIGSEGV
SIGINT
SIGFPE
SIGHUP/SIGPIPE
SIGUSR1 and SIGUSR2
SIGKILL
SIGTERM
SIGSTOP

## The Linux signal API
`sigaction` -- register signal handler
`kill` -- send signal to process
    * `raise` -- send signal to self 
`pause` -- put process to sleep until signal received 
`sigprocmask` -- temporarily block or unblock some signals from being received
    * signal will be _pending_ until unblocked, at which point the signal will be _received_


