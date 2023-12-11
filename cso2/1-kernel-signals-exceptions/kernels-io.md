# Kernel Mode vs. User Mode

Modern chips mark out at least two levels of privilege commonly simplified to _user mode_, where our regular code runs with notable restrictions, and _kernel mode_, where the most essential, trustworthy code runs essentially unrestricted.

## Defining the Modes

**Kernel mode** allows running software to run any instruction on the hardware that the hardware supports. Code in **user mode** introduces guardrails that prevent regular, unprivileged programs from modifying certain system details, data, and behavior.

Code running in user mode that wishes to perform a kernel mode task like printing to the screen or reading input must make a request of the kernel, most commonly done through a _system call_ which triggers the kernel to run.

## Motivation

With user mode restrictions come advantages.

### Limiting Bugs' Potential for Evil

Simply put, we would prefer that a Hello World program not crash our operating system or brick our hardware. The kernel is incredibly sensitive software handling the tasks that keep our computer running at the basic level. We should try to separate it from standard user code as a responsible security measure--in user mode, unexpected behavior might cause the process to crash, but is unlikely to crash the entire operating system.

### 1.2.2 Each Program Thinks it is in Control

While our computers are subject to many limitations (memory constraints, cores and threads avilable, etc.), our user code doesn't strictly have to worry about that. The kernel and hardware work together to "fake out" user mode code with the illusion that these limits do not actually exist. 

Limits become clearer when we run some or even many processes simultaneously (as is common on today's computers). One of your processes (say, your terminal) should not be able to simply see the memory of another process (say, a text editor) at will. Sophisticated kernels isolate these processes from one another so that each is given a "virtual machine" that a user mode process can run on without worrying about complications like resource management, which the kernel will handle instead.

### Wrapper Around Hardware
It would get very tedious (and perhaps _dangerous_) to have to keep track of every driver for every mouse and keyboard when we write user mode code as application programmers. The kernel worries about protocols at the fine-grain level so we don't have to.

### Multiple Users
The kernel intercepts _everything_ which would interact with critical system resources. If I try to access another user's disk space, for example, the kernel can intervene and prevent such an action to keep user data separate and secure.

## Implementation

### Protected Instructions and Memory

Certain instructions cannot be executed in user mode, and certain memory (kernel memory) cannot be read in user mode. We are not able to, for example:
* Access memory about our account information
* Run instructions to talk directly to I/O devices
* Run instructions to change our own state to run in kernel mode

### Mode Bit

A simple way to implement a user mode/kernel mode separation would be to add a single-bit register on-chip that protected instructions check first, and then raise exceptions if the register indicates user mode. In reality, multiple levels of privilege exist to allow for more sophisticated privilege structures.

### Mode-Switch via Exceptions

Most commonly, we switch to the kernel due to a hardware exception, which allows processer state changes like changing to kernel mode and jumping to kernel memory to start executing kernel code. Entering kernel mode necessarily runs the kernel itself, which precludes executing user code in kernel mode.

# Typical I/O Patterns

## Input

Multiple exceptions and context switches can be involved when a program receives input. The following is a typical pattern:
1. A program invokes a system call to request the OS to receive input
2. The OS may have input immediately on hand, in which case the OS can return that input data to the system call handler program.
3. Otherwise, since the program depends on input data to continue running, the OS will context switch to something else to keep the processor "useful" (i.e., optimize time multiplexing and limit CPU idle time) until input becomes available.
4. The device from which input is requested will trigger an interrupt to alert the OS to new input availability. The OS will copy the input data and return the data to the program which needed the input. The OS can either context switch immediately to the original program or schedule the program to run again at some later time.

## Output

Typical output patterns are much like typical input patterns:
1. A program will request that the OS sends output to a device
2. The OS will often be able to send output to a device immediately, though the OS may need to wait until the device is ready. 
    1. Depending on whether the program must wait until the output operation is complete or not, the OS will continue running the program or perform a context switch to reduce CPU idle time.
3. If the device was not immediately ready to receive output, the device can trigger an interrupt to indicate that it is ready to receive more output. The OS will handle the interrupt and complete the output operation.

