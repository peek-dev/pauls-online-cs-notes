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

