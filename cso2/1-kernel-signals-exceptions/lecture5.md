**Lecture 5: September 5, 2023**

# Last time
* Context switch: save current context and restore old context
    * _Context_: processor state (registers, program counter, etc.)
* Thread: illusion of own processor
    * Independent state (PC, registers, etc.)
    * Implemented by sharing processors to support time-sharing registers
* Process: thread(s) and address space ("own machine" abstraction) as if separated from other programs
* Hardware:(Unix OS)::exceptions:signals
    * Signals are the POSIX version of exceptions. The hardware generates exceptions; POSIX generates signals
    * OS call signal handlers to interrupt thread
    * OS "asks program for help" by (in many cases) forwarding the original exception as a corresponding signal (e.g, `SIGSEGV` for paging/protection fault)

# Anonymous feedback
_Lots_ of anonymous feedback about quizzes.

In the kernel reading, there was a _system call_ for _reading_ the keypress, but the _processor_ receiving the keypress is a _non-system call exception_. 
* The question here: what triggered it?
    * Start waiting on input: program begins this by asking the OS (a system call)
    * Processor receives keypress and forwards this to OS (hardware "starts" this): _non-system call exception_.

## Quiz Review
Keep in mind that you can still access the user stack from kernel mode. It's thus not necessary to "feed" arguments given to `printf` through processing and generate a fixed-format string of arguments to call `write` (or a direct syscall wrapper) with.

# The POSIX signal API
* `sigaction`: register handler for specific signal
* `kill`: send signal to process (by process ID)
* `pause`: put process to sleep until signal received
* `sigprocmask`: temporarily block or unblock some signals from being received
    * signal will be acknowledged but _pending_ until unblocked (when it is fully received)
    * useful for not needing to handle a certain signal right away
* ...and much more

## `kill`
In shell, `kill` (1) calls `kill()`:
* `kill 1234`: SIGTERM to pid 1234
* `kill -USR1 1234`: SIG**USR1** to PID

## `SA_RESTART`
* `SA_RESTART`: attempts to restart interrupted action once signal handler runs
* Without `SA_RESTART`: returns error number status for interrupted action once signal handler runs

# What's the output?

```c
void handle_usr1(int num) {
    write(1, "X", 1);
    kill(2000, SIGUSR1);
    _exit(0);
}

void handle_usr1(int num) {
    write(1, "Y", 1);
    _exit(0);
}
```

# x86-64 Linux signal delivery
* `__restore_rt` for "sigreturn" syscall
* Signal handler _unsafety_
* How can you restart an interrupted operation? -- the OS runs to do that

## Signal handler _unsafety_
```
void foo() {
    
}

void handle_sigint() {
    printf("Ctrl-C pressed?!");
}
```

# The signal lab
You'll be writing a **signal-based chat** which operates between two running programs _A_ and _B_:
* _A_ features a signal handler that prints from shared memory
    * `main() { while(1) { readline(); write to B's shared; signal B;`
* _B_ also features a signal handler that prints from main memory
    * `main() { while(1) { readline(); write to A's shared; signal A;`


