CSO2 explores important topics which develop a more realistic model of processors. We'll remove the "black box" from a computer's operating system that CSO1 previously placed and take a look at the interaction between the OS (specifically the _kernel_) and hardware which constitutes the foundation of contemporary computers.

# Interrupts: safe communication with hardware

Consider the following question:

> How do I collect user input when they press a key?

A first approach might check in a loop whether a key was pressed, then interpret the key press and react accordingly. This approach--**polling**--is a good first method, but we do have better ways. Polling also has its costs: checking user input in a loop pauses the program execution for an indeterminate amount of time until that input is received and verified.

Our processors afford us a better approach using **interrupts**. Whenever an event like receiving a key press, receiving a packet over the network, or connecting an external device to our computers occurs, interrupts are sent to the processor, and the processor reacts accordingly. Interrupts trigger special privileged code known as **interrupt handlers** which the processor begins executing as soon as possible after it receives the interrupt.

Relying on interrupts saves us time and resources when we go to write our user code. Instead of having to figure out how to check for asynchronous events, we can trust the processor to:
* Have specialized facilities to handle events
* Run trustworthy code to respond to events as soon as they occur and in a way that can be ultimately useful to us

Interrupts and interrupt handlers remove the need for us to check input ourselves in user code and provide a secure separation between our user code and _driver code_ which communicates between peripheral devices and the processor.

Generally, interrupt handling falls under the **exception handling** umbrella. The operating system jumps in to respond to many types of _exceptions_:
* **Interrupts** (I/O device/hardware events)
* **Faults** (unsuccessful instructions like divides by zero or illegal memory accesses)
* **Hardware timer exceptions** (process runs too long), which help ensure the processor is shared between other programs which need to run (processor _multiplexing_).

# Virtual Memory: the illusion of isolation

Previously, we didn't worry about where _other programs' memory_ was stored in relation to ours. CSO1, for instance, relied on a "one big array" model of memory which placed our local variables on the stack; our larger variables on the heap; our code in "the code section"; our global variables in "the global variables section"; and other programs' memory..._somewhere_.

For good reasons like security, computers actually separate our programs' "views" of memory from one another using a suite of tools we call _virtual memory_. Virtual memory is essentially an indirection tool to provide our programs the illusion that they have "their own view of memory". The central tool here is **address translation** which maps the addresses our programs see (_virtual addresses_) to the actual locations "on-chip" where data is stored (_physical addresses_). 

In standard assembly, we can consider the following memory access:
```x86asm
movq	20(%eax), %ebx
```

Which could be written in C-like pseudocode as:
```c
ebx = memory[20 + eax]
```

But which in reality happens on our machines as:
```c
ebx = memory[translation[20 + eax]]
```

Here, `translation` is that address translation mechanism we alluded to earlier. How do we get this?
* Providing a hardware-readable data structure
* Granting privilege exclusively to special code (the operating system's **kernel**) to modify memory
* Optimization to make virtual memory translations and accesses competitive with direct memory access (DMA) in terms of efficiency

Virtual memory is an excellent example of the cooperation modern computers rely on between "the operating system" (the kernel) and "the hardware" (processors, memory, etc.) to provide vital functionality our user programs ultimately depend on.

# Networking: connecting to other computers

Computer networking is a ubiquitous process in contemporary computing. Most commonly, we use DNS (Domain Name System) and TCP/IP (Transport Control Protocol-Internet Protocol).

# Caching: avoiding repetitive work

> What happens once is likely to happen again.

If we perform a computation by going through all the mathematical steps once, we would like to not have to repeat all those manual steps, at least for a time. Computers thus take advantage of the above principle through **caching**: storing recent data in a small, quick-access space which shares the chip with the processor itself. If the processor needs the stored data again, the processor can pull from its cache instead of having to recompute the data or perform the translation and similar procedures required to pull the data from main memory.

This is important since drawing from cache instead of main memory can reduce the access time required by up to two orders of magnitude. To take full advantage of this, though, we as programmers need to pay close attention to how we write our code. Specifically, we'll want to write code with good _temporal_ locality (accessing the same data over multiple instances grouped close to one another in time) and good _spatial_ locality (accessing data which we would expect to be stored closely together in memory, and therefore cache, such as an array). Caching also has security implications worth considering related to "side channels": artifacts of computation that attackers can see and piece together to compromise program security.

# Synchronization: concurrency requires coordination

As end users, we often want our computers to perform multiple tasks simultaneously. For example, if we open multiple tabs in a browser application, we would like each tab to load webpages at roughly the same time without needing to "take turns" to fetch their respective content.

We achieve simultaneous computation through one or both of two important ways:
* **Concurrency**: the _appearance_ of simultaneous computation in which a processor jumps back and forth between tasks quickly enough that a human end-user does not notice. One can consider this a "persistence of vision" effect where the processor swaps out which program "has its turn" to compute quickly and efficiently enough (_context switches_) that programs appear to run at the same time.
* **Parallelism**: actual simultaneous computation in hardware where a single processor unit (a CPU) has multiple physical cores that have the full set of tools required to execute a program. When executing _in parallel_, multiple processes can execute truly simultaneously.

With this power comes the responsibility to coordinate, however. We can encounter various problems with _synchronizing_ across concurrency or parallelism very quickly. We'll consider these questions when trying to achieve good synchronization:
* How do we wait for another agent?
* What if two agents are mutually waiting on the other to act?
* How does this play out in hardware and software?

# Processors: idle = wasteful

Like with human brains, we aren't generally likely to be using all available faculties at once. A processor has many specialized subsystems, but our instructions may only be using an adder or a multiplier at a single moment in time. Just because we're using only some parts of the chip doesn't mean there isn't cost, however: idle transistors, gates, and other hardware still incur power costs.

In many cases, we also need some parts of an instruction to finish up before we can execute other parts of that instruction which depend on previous results. This waiting also incurs costs and can slow our programs down at runtime.

Like an assembly line in manufacturing, processors divide up work into sequential parts using **pipelining**. This increases _throughput_ (instructions completed per microsecond) but does also increase _latency_ (picoseconds needed for one instruction to complete). We accept this trade-off as it stands since throughput is often more important to us.

Because of the aforementioned dependency on previous data, however, we run into complications with pipelining that mean it's not a perfect fix. We can try workarounds like _forwarding_, taking information from a previous incomplete instruction to "fill in" a gap for a following instruction; _out-of-order execution_, picking an instruction later in sequence but with fewer dependencies and executing that first; or _speculative execution_, making a "best guess" at junctures like conditional branches, executing the code that "best guess" points to, and discarding results if the guess was in fact incorrect.

These workarounds are not only imperfect, but can in fact be dangerous. Exploits like Spectre and Meltdown (from 2017-2019) play on security weaknesses in speculative execution to access data that otherwise should be private and inaccessible. Despite fixes rolling out for these previous vulnerabilities, newer ones like Downfall and Inception (from 2023) demonstrate that more weaknesses can emerge with speculative execution.
