# 1. Kernel Mode vs. User Mode

Modern chips mark out at least two levels of privilege commonly simplified to _user mode_, where our regular code runs with notable restrictions, and _kernel mode_, where the most essential, trustworthy code runs essentially unrestricted.

## 1.1 Defining the Modes

**Kernel mode** allows running software to run any instruction on the hardware that the hardware supports. Code in **user mode** introduces guardrails that prevent regular, unprivileged programs from modifying certain system details, data, and behavior.

Code running in user mode that wishes to perform a kernel mode task like printing to the screen or reading input must make a request of the kernel, most commonly done through a _system call_ which triggers the kernel to run.

## 1.2 Motivation

With user mode restrictions come advantages.

### 1.2.1 Limiting Bugs' Potential for Evil

Simply put, we would prefer that a Hello World program not crash our operating system or brick our hardware. The kernel is incredibly sensitive software handling the tasks that keep our computer running at the basic level. We should try to separate it from standard user code as a responsible security measure--in user mode, unexpected behavior might cause the process to crash, but is unlikely to crash the entire operating system.

### 1.2.2 Each Program Thinks it is in Control

While our computers are subject to many limitations (memory constraints, cores and threads avilable, etc.), our user code doesn't strictly have to worry about that. The kernel and hardware work together to "fake out" user mode code with the illusion that these limits do not actually exist. 

Limits become clearer when we run some or even many processes simultaneously (as is common on today's computers). One of your processes (say, your terminal) should not be able to simply see the memory of another process (say, a text editor) at will. Sophisticated kernels isolate these processes from one another so that each is given a "virtual machine" that a user mode process can run on without worrying about complications like resource management, which the kernel will handle instead.

### 1.2.3 Wrapper Around Hardware
It would get very tedious (and perhaps _dangerous_) to have to keep track of every driver for every mouse and keyboard when we write user mode code as application programmers. The kernel worries about protocols at the fine-grain level so we don't have to.

### 1.2.4 Multiple Users
The kernel intercepts _everything_ which would interact with critical system resources. If I try to access another user's disk space, for example, the kernel can intervene and prevent such an action to keep user data separate and secure.

## 1.3 Implementation

### 1.3.1 Protected Instructions and Memory

Certain instructions cannot be executed in user mode, and certain memory (kernel memory) cannot be read in user mode. We are not able to, for example:
* Access memory about our account information
* Run instructions to talk directly to I/O devices
* Run instructions to change our own state to run in kernel mode

### 1.3.2 Mode Bit

A simple way to implement a user mode/kernel mode separation would be to add a single-bit register on-chip that protected instructions check first, and then raise exceptions if the register indicates user mode. In reality, multiple levels of privilege exist to allow for more sophisticated privilege structures.

### 1.3.3 Mode-Switch via Exceptions

Most commonly, we switch to the kernel due to a hardware exception, which allows processer state changes like changing to kernel mode and jumping to kernel memory to start executing kernel code. Entering kernel mode necessarily runs the kernel itself, which precludes executing user code in kernel mode.

# 2 Exceptions

_Exceptions_ in name can be considered with respect to control flow (_exceptions_ to standard sequencing and jumps) and to expected situations (_exceptional_ situations).

## 2.1 Kinds
Terminology varies, but these are CSO2 definitions.

### 2.1.1 Interrupts
These are asynchronous events that, once noticed, trigger interrupt handler code within the kernel that is well-separated from user code. Interrupt handlers run as soon as possible and "freeze" user processes in place before interpreting the interrupt and resuming the previous user process.

### 2.1.2 Faults
These are calls for assistance raised when instructions are unsuccessful. These could be related to arithmetic (zero division), memory access (null pointers or general _page faults_), or privilege (attempting to run kernel-mode code in user mode).

### 2.1.3 Traps
These are deliberate, dedicated exception-raising instructions designed to prompt a switch to kernel mode. Traps are typically wrapped in various _system calls_ that, when invoked by user mode code, trigger the kernel to actually carry out the work involved for tasks like file (and file system) interactions, thread creation and waiting, and reading input.

User-mode code itself has a _very limited_ direct interface with the hardware. Instead, user-mode code has far more ability to interact with the _system call interface_ which run the kernel under controlled conditions; the kernel itself has complete and unrestricted permission to interface with hardware. 

## 2.2 Handling
The processor handles an exception by switching to kernel mode and executing an _exception handler_ in kernel memory. Exceptions are often _asynchronous_ and can therefore occur at any point in code execution. To properly guarantee a save-and-restore pattern for a running process, the exception handler also saves processor state (program registers, stack, etc.) before running the rest of the handler.

### 2.2.1 Save-Handle-Restore
Specifically, the flow of an exception handler is:
1. Save aspects of processor state like program counter, registers, and kernel mode bit
2. Enter kernel mode
3. Jump to specialized **exception handler** dealing with the given exception, which is located in kernel memory
4. Enter user mode and restore the previous processor state (program counter, kernel mode bit, etc.) once exception handling has finished. 

With the exception of when the actual "substance" of the exception handler is running (i.e., any portion not between starting formal exception handling in step 3 and finishing up in step 4), all steps happen atomically. Saving the state of the processor, jumping to kernel mode, and jumping back to user mode cannot be interrupted, but the actual exception handling may be halted upon recording another exception.

### 2.2.2 Which Handler?
There are many exception _causes_ which there must be many _handlers_ for. The hardware consults an **exception table** to decide which exception maps to which handler. This is simply an array of code addresses with which an exception number corresponds to an index (the **exception number** or **vector number**).

---

#### Considering the Exception Table

The exception table is much like a `switch` statement in C which would compile to a "jump table" of addresses in assembly.

Here's how the switch statement model of an exception table might look:
```c
switch(x) {

    case 0: f0();
    case 1: f1();
    case 3: f3();
    case 2: f2();
        break;
    case 4:
    case 5: f5();
    case 6: f6();
        break;
    case 8: f6();

}
```

And in assembly, here's how that might compile:
```x86asm
    cmpl    $8, %eax
    ja      AfterSwitch
    jmpq    * Table(,%rax,8)
Case0:
    callq   f0
Case1:
    callq   f1
Case3:
    callq   f3
Case2:
    callq   f2
    jmp     AfterSwitch
Case5:
    callq   f5
Case6:
    callq   f6
    jmp     AfterSwitch
Case8:
    callq   f6
AfterSwitch:
```

With a corresponding jump table in assembly of:
```x86asm
    .section    .rodata
Table:
    .quad   Case0
    .quad   Case1
    .quad   Case2
    .quad   Case3
    .quad   Case5
    .quad   Case6
    .quad   AfterSwitch
    .quad   Case8
```

Again, this really constitutes an array of code addresses (often _function pointers_) dressed up in a fancy way.

---

### 2.2.3 After the Handler
Handlers have the discretion to resume user code or to abort user code if the exception is unrecoverable.

The handler will have slightly different return behavior depending on the exception's original cause. 
* A _trap_ is much like a `callq` and the corresponding handler would resume executing at the _next_ following instruction. 
* A _fault_, however, is generated as a result of an instruction failing, so a handler would attempt to rerun the original triggering instruction so that it might succeed on another try. 
* _Interrupts_ are asynchronous events and would simply have the handler jump back to user code at the next instruction "like nothing happened"
* Lastly, an _abort_ would stop running instructions in both normal cases (program invocations of the `exit` system call) and abnormal cases (unrecoverable memory inconsistency/hardware issues).

---

#### Nested Exceptions

It is possible for exceptions (especially interrupts since they are asynchronous) to occur _while another exception is being handled_. It's therefore difficult to write exception handlers to cooperate with other exception handlers should the original handler be interrupted. Fortunately, processors and their instruction sets often provide a way of disabling interrupts so that exception handlers may run atomically for their most important portions or for their entirety. 

Conceptually, this would be like:
```c
void HandleCertainException(void) {

    DisableInterrupts(); // often a wrapper for some specific assembly instructions
    
    // atomically run interrupt handling code goes here

    EnableInterrupts();

    return;

}
```
---

### 2.2.4 Linux system calls
In Linux, user-to-kernel code communication gets assigned a trap with exception/vector number 128. The exception number is not directly used to select which system call gets run; rather, a number is first placed into `%rax` to select the system call before the general `syscall` trap instruction is executed. 

Linux system calls are given at `man 2 syscall` for reference. There are many system call _wrappers_ in the C standard library as listed in `man 2 syscalls`.

We can look at the `socket` system call wrapper, part of the C standard library, to illustrate general system call and system call wrapper behavior.

---

#### Example with `socket`

Here's the `libc.so` assembly for `socket`:
```x86asm
socket:
    endbr64
    mov     $0x29, %eax
    syscall
```

First, `endbr64` performs _control-flow enforcement_ that prevents malicious code from potentially executing the `syscall` instruction (and jumping to an unexpected system call) without first setting `%rax` in the instruction immediately preceding `syscall`.

`mov $0x29, %eax` places 0x29 (decimal 41) into `%rax` to select the `socket` system call in accordance with `socket`'s syscall number.

Lastly, `syscall` is the dedicated _trap_ which generates a 128 exception number. The hardware saves the processor state, switches to kernel mode, and jumps to kernel memory at index 128 of the exception handler.

This exception handler will check `%rax` again for validity, then jump to a system call handler also in kernel memory at index 41. Here lies the actual implementation of `socket`.

Lastly, the handler wraps up with a protected, atomic instruction that:
* Restores the processor state except for `%rax`, which is preserved as a return value for the system call that was just executed.
* Returns to user mode
* Jumps to user code 
    * **Questions**: (so it's at a kernel address in user mode _before_ jumping back to a user address? Is this better/more secure than the other way around, where the kernel jumps to a user mode code address _and then_ switches back to user mode?)

This makes the exception handler quite efficient even while the `socket` implementation residing at index 41 of the system call handler is complex.

---

#### System Libraries and System Call Wrappers

System call wrappers must be used in C since C itself doesn't provide functionality for direct system calls (these must happen in assembly). The wrapper is therefore a quick shorthand for the assembly required to actually make the system call. System libraries provide these wrapper functions for code to use at will.

System call interfaces are quite low-level even with the abstraction that wrappers provide. Often, there are higher-level functions like `malloc` and `printf` that are more accessible to end users and application programmers, but do so by being implemented in library code that makes simpler system calls.

---

## 2.3 Exception-Like Constructs

### 2.3.1 Signals

Signals allow kernel-to-user code communication. The following table also lists exception types and communication methods:

|   | **TO User code** | **TO Kernel code** | **TO Hardware** |
| --- | --- | --- | --- |
| **FROM User code** | ordinary code | Trap | _kernel_-mediated |
| **FROM Kernel code** | Signals | Ordinary code | Protected (kernel-mode only) instructions |
| **FROM Hardware** | _kernel_-mediated | Interrupt | -- |

Like how interrupts can occur from the _hardware_ to the _kernel_ at any time ("asynchronously"), signals can occur from the _kernel_ to _user_ code at any time. If a user code receives a signal, the code will suspend its normal operation, run a special _handler_ to deal with the problem, and return to normal code if the problem has been successfully resolved. 

Linux defines signals in `man 7 signal`, listed by name (and signal number, since names really reflect integer constants) as well as default action if no suitable handler exists, which is generally to terminate the process.

Here's an excerpt from `man 7 signal`, lightly annotated for clarity:

| Signal name | Default action | Description |
| --- | --- | --- |
| SIGFPE | Core dump (termination) | Floating-point exception (e.g., modulo by 0) |
| SIGINT | Terminate program | Interrupt from keyboard (Ctrl+C or similar) |
| SIGKILL | Terminate program | Kill signal (like "force quit"). **Cannot be handled** and will immediately terminate program. |
| SIGSEGV | Core dump | Invalid memory reference or access (e.g., _page fault_ or _protection fault_) |
| SIGSTOP | Stop | Stop process. **Cannot be handled.** | 
| SIGTSTP | Stop | Stop typed at terminal (Ctrl+Z or similar). **Cannot be handled.** |
| SIGUSR1 | Terminate program | User-defined signal 1 |
| SIGUSR2 | Terminate program | User-defined signal 2 |

---

#### A signal handler example

```c
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// standard signal handler function prototype/header
static void handler(int signum) {
    // signal-safe way to print message
    write(1, "Provide end-of-file (Ctrl+D) to end program.\n", strlen("Provide end-of-file (Ctrl+D) to end program.\n"));
}

int main() {

    /**
    * An OS-readable data structure with multiple fields to dictate:
    *   What we want to handle; 
    *   Where (i.e., the function) we want to to do the signal handling, and;
    *   How we want to return from signal handling if possible.
    */
    struct sigaction act;

    act.sa_handler = handler; // signal handler address
    sigemptyset(&act.sa_mask); // do not block additional signals while handler is running. Means handler can be interrupted by another signal.
    act.sa_flags = SA_RESTART; // restart functions that can recognize and recover from interruption

    sigaction(SIGINT, &act, NULL); // register signal handling information (struct sigaction container) with OS. Handler should trigger on SIGINT only.

    ... 

}
```

Signals can be used for inter-process communication, though they offer blunt, limited ways of communication when compared to other methods like sockets and pipes. Generally, we would use sockets or pipes in cases where we want to establish more sophisticated inter-process communication, especially when sending and receiving data between processes.

The system call to send a signal to another process is `kill`, which may be invoked in the shell (`kill(1)`) or which may be invoked as a system call wrapper in C (`kill (2)`).

---

#### Signals on the command line

The standard Linux shell (`bash`) allows sending signals to processes as covered in `man 1 kill`.

Examples:
* Ctrl+C sends SIGINT (interrupt signal).
* Ctrl+Z sends SIGTSTP to suspend the process by default.
* `bg` resumes a suspended process, but in the _background_ (i.e., process will be detached from the terminal when resumed).
* `fg` resumes a suspended process in the _foreground_, which reattaches the keyboard to `stdin`.
* `kill <pid>` without any other arguments sends SIGTERM to process `<pid>`. SIGTERM can be handled.
* `kill -9 <pid>` or `kill SIGKILL <pid>` sends SIGKILL to process `<pid>`. SIGKILL parallels a "force quit" and cause the program to terminate immediately.

---

# 3. Processes and Threads

A **process** is a unit of software in an operating systems context that parallels a running program. A process has the full suite of tools (stack, heap, globals and read-only, address translation, core(s) and thread(s), etc.) required to execute code on a computer. The process classification is all the operating system really needs to make, since differences between programs (e.g., a web browser and a terminal) can be simplified as "different processes having different needs". 

Processes essentially run in "virtual machines" with their own memory, processor cores, and more. Behind the scenes, the OS maintains data structures within kernel memory to map processes, their memory, and state to the code that will run.

## 3.1 Threads

Each "processor core" (discrete computational unit) within a process is known as a **thread**, where each thread has an independent slate of program registers, a program counter, condition codes, and more state. Given this, threads can change their independently allocated state without concern for other programs' states (**thread safety**).

The operating system loads a thread's values and state into processor registers and appropriate condition codes.

## 3.2 Virtual address spaces

Each process's "view of memory" is more formally a _virtual address space_ which is maintained independently of other processes' address spaces; like with individually allocated thread state, each process can modify memory within its address space at will without worry for other programs' states.

Virtual memory encompasses the tools which implement virtual address spaces, often through _address translation_ and the use of special kernel-maintained data structures like page tables.

## 3.3 Context Switches

Changing the thread or process state loaded onto a processor core is a **context switch**. Major _time-sharing_ operating systems (Windows, MacOS, Linux) use hardware timers to create exceptions for _automated_ context switching regardless of how long it takes threads and processes to actually conclude their comptutation. 

Automated context switching also facilitates concurrency; quick and smooth context switching creates the illusion that more processes are running on the computer than it has space for, strictly speaking (with respect to physical limits on cores and threads).

# 4 Typical I/O Patterns

## 4.1 Input

Multiple exceptions and context switches can be involved when a program receives input. The following is a typical pattern:
1. A program invokes a system call to request the OS to receive input
2. The OS may have input immediately on hand, in which case the OS can return that input data to the system call handler program.
3. Otherwise, since the program depends on input data to continue running, the OS will context switch to something else to keep the processor "useful" (i.e., optimize time multiplexing and limit CPU idle time) until input becomes available.
4. The device from which input is requested will trigger an interrupt to alert the OS to new input availability. The OS will copy the input data and return the data to the program which needed the input. The OS can either context switch immediately to the original program or schedule the program to run again at some later time.

## 4.2 Output

Typical output patterns are much like typical input patterns:
1. A program will request that the OS sends output to a device
2. The OS will often be able to send output to a device immediately, though the OS may need to wait until the device is ready. 
    1. Depending on whether the program must wait until the output operation is complete or not, the OS will continue running the program or perform a context switch to reduce CPU idle time.
3. If the device was not immediately ready to receive output, the device can trigger an interrupt to indicate that it is ready to receive more output. The OS will handle the interrupt and complete the output operation.

