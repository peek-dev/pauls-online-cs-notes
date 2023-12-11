# 1. Introduction and Motivation

**Virtual memory** arises from hardware/software cooperation and provides the following benefits:
* Each process on the system has "its own view of memory", i.e., the illusion of _process isolation_
* Each process operates with the illusion that it can access _all memory addresses_ regardless of how much physical memory is actually available
* Memory protection is enforced so that user-mode code _cannot_ access kernel memory that would be potentially unsafe to handle

# 2. Memory Regions

Under virtual memory, accesses are checked against a set of permissions to enforce process separation and related protections. Common protections and distinctions include:
* Read-only memory (constants, etc.) from read-write, or _writable_, memory
* User-mode memory ("normal" data on stack, heap, global, etc.) from kernel memory (exception handlers, private system details, etc.)
* Executable memory (code) from data memory (not code, so it _should not_ be executable)
* Valid addresses from invalid addresses (e.g., the "zero page" which a null pointer references)

## 2.1 Segmentation

_Segmentation_ is the practice of mapping memory in variable-sized _segments_ to make memory regions available to processes. This is one of the primary options that a user can select as a memory management technique in today's processors (along with _paging_, mapping memory in fixed-size _pages_, which will be developed extensively later). Generally, user-mode code can use a high-level _software-enforced_ version of the segmentation idea to request memory from the operating system[^1] using system calls like `mmap` and higher-level wrappers like `malloc` and `posix_memalign`.

Since segmentation in the most common form today is merely software-enforced, the actual memory management technique that today's operating systems cooperate with hardware to provide is _paging_, the provisioning of physical memory in fixed-size _pages_. Thus, the actual hardware implementation relies on _paging_, and _segmentation faults_ are in reality responses to either a _page fault_ (access to an invalid page in memory) or a _protection fault_ (improper type of access to a particular memory space violating permissions like read-only or kernel-only).

## 2.2 Pages

_Paging_ relies on hardware-friendly data structures and access patterns to balance security with virtual memory and efficiency in memory accesses. Most commonly, operating systems maintain a data structure called a _page table_ that stores memory mapping information for one or multiple levels.

[^1]: On a Unix-like system, particularly Linux.

