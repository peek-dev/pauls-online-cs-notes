
# Privilege

## Things you shouldn't be able to do on portal

If we use the CS department servers, for instance, this is a _shared resource_ that needs to be safe and available for others to use.

Since we are normal student users, we **should not be able to**:
* Read other users' files
* Modify the server's OS memory
* Read other users' data into memory
* Hang the entire system with an infinite loop or the like

This collides...well, _inconveniently_...with how we're used to thinking about programs running on a machine. How is our OS able to know who is running a process? Isn't code just loaded into _the same place_ in memory and executed irrespective of who wrote it?

## Privileged operation problem

How can your hardware and the operating system read your own files from disk but prevent you from reading others' files or, really, "doing the reading yourself"?

The operating system _could_ set up the read for the hardware by marking out parts of the hard drive _before_ handing off code execution to the hardware. This is _incredibly_ tedious, however.

Another approach might be that the OS verifies what kinds of accesses your program's code should and shouldn't be able to do. Operating systems don't do this, though, since this is also quite tedious and difficult to tell what machine code is doing.

Our solution is **the kernel**: the OS tells hardware that only OS-written code should access the hard drive. The kernel is a single entry and exit point for file reads, allowing the OS to consistently enforce permissions while requiring only relatively simple interpretation from hardware.

# Kernels

## Kernel mode

We can delineate between a regular user and the operating system by keeping a simple one-bit register that asks "are we in kernel mode?". If we are in kernel mode, we have significant _privilege_. If we aren't in kernel mode, we are in _user mode_, and cannot directly perform many sensitive operations like interacting with hardware.

This does, though, require some extra overhead to set up transitions between kernel mode and user mode. The boot process will place the computer in kernel mode initially, and the kernel will be able to jump into user mode once all necessary operating system tasks to start itself up have completed.

## Enforcing kernel mode

So, how do we jump from user mode to kernel mode? User-mode code has access to the _system call interface_ that allows code to make a direct _request_ to the operating system to have sensitive operations performed on the user-mode code's behalf without the user-mode code having to worry about the operation itself.

Again, we _can't_ really directly call the kernel. This would be quite difficult to implement and _super dangerous!_. Instead of a many-to-many mapping where user programs can call many kernel functions, all requests to the kernel (system calls) are routed through the special `syscall` function. The process is thus:
1. User program executes `syscall` instruction, which switches the machine into kernel mode and hands off control.
2. The initial portion of `syscall` will enforce permissions and the validity of the parameters it is given.
3. Once validity, permissions, and security has been verified, `syscall` will route the system call to the appropriate implementation in the kernel code based on the parameters originally passed to `syscall`.
    1. `syscall` takes the value of register `%rax` for the system call number and the registers `%rdi`, `%rsi`, etc. for arguments to the function.
    2. The `syscall` internals will check `%rax` and map to the right system call **in that order**. The `syscall` implementation includes a specific instruction `endbr64` to enforce strict, complete, sequential execution and prevent code from making wild jumps.

Again, we typically see a _multi-layer check_ to ensure at each step that the system call is valid and occurring under an originally appropriate level of privilege (i.e., that nothing unexpected occurred to improperly change state during the system call). This re

## Hello, world! in assembly
Why `_start` instead of `main`? Programs start executing on compiler-generated functions, which _then_ pass off control to `main`. We use `_start` because we _don't_ want to link against the C library (and therefore need to assemble with an option to disable linking with `libc`).

## System Call Examples

`mmap`, `brk`

## System call wrappers
We can often "make system calls" with the C standard library without _actually_ doing so by using "system call wrappers." For example, the `open()` function in the C standard library actually _wraps_ (i.e., hides internally and abstract from the user) a system call which is actually carried out with an inline `syscall` instruction in assembly as the code runs. Colloquially, we might refer to functions like `open()` as "system calls" even in C as a result. 

### Tracing system calls with `strace`

The `strace` tool is a Linux utility to trace system calls that traces system calls step-by-step over a program lifetime much like a debugger. Even a program that is simple _to us_ like a short "Hello, world!" progrma in C can involve _many_ system calls (on the order of 10 or similar).

## Complicating the System Call Interface
When we typically write programs, we're actually dealing with a few more layers of indirection. We might refer to an intermediate layer between our application and the system call interface as the _library interface_ which connects function calls to _system libraries_ that hold the code for higher-level functions (`printf`, etc.).

Now, what's this _limited_ user-mode to hardware interface? We _can_ write `mov`, `push`, `pop`, etc. without making system calls as this interface.

# Memory Protection

Can we modify another program's memory? _Should we?_

No and _probably not_. We use indirection with _address spaces_ (one of the tools in a **virtual memory** suite that is OS-maintained and OS-determined) to ensure that even the "same" addresses that different programs "see" _aren't actually the same places in hardware_.  Each time the OS launches a new program, part of that launch process necessarily includes "spinning up" a new mapping.

But what if two processes need the exact same spot in memory? Well, that's a good cause for _more system calls!_

And the procesor _maintains_ this mapping physically on _every single instruction that interacts with memory_. So, a memory move would actually need to do address translation _twice!_ (One for the instruction; the other for the data).

What about direct memory access? (DMA). For consumer operating systems on phones or desktops, _no_.

## Memory Errors

What happens on the processor when a program crashes? We'll use the processor to interpret the error and then hand off control to a dedicated OS routine (much like a system call!).

# Exceptions

We can generalize this function to the problem of the hardware calling the OS when the hardware needs extra help.

System calls -- _software_ asks OS for help.
Exceptions -- _hardware_ asks OS for help (more generally).

## Exception Types

* System calls -- intentional ("traps")
* Errors and unexpected or synchronous events
    * Out-of-bounds memory ("segmentation fault", or "paging" fault to be more technically accurate)
    * Privileged instruction
    * Divide by zero or invalid instruction
* Asynchronous events
    *  Interrupts: key presses, memory parity errors, packet reception, etc.
* Timing
    * Limiting the maximum amount of time that a program can run at once
    * _Hardware timer interrupts_ or _OS timer interrupts_ cue the OS to preempt a running program and run a kernel routine.

## Nested exceptions

Exceptions can happen **at any time** while any program is running, even potentially other exception handlers!

We can often execute special instructions (in inline assembly, never in C or another higher-levellanguage) to temporarily disable interrupts/exceptions. In practice, exception handlers often do this so that they may run "atomically" (i.e., in a way that _cannot_ be interrupted before completing initial handling). 

