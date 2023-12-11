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

_Paging_ relies on hardware-friendly data structures and access patterns to balance security with virtual memory and efficiency in memory accesses. Most commonly, operating systems maintain a data structure called a **page table** that stores memory mapping information for one or multiple levels. Like with "data memory" (e.g., part of the stack in a user process), page tables occupy exactly one page, even if mappings the page table stores may not all be valid. The advantage of paging over segmentation is that memory and associated data structures are more easily partitioned based on the agreed-upon page size. When designed well (particularly on modern operating systems), page tables involve a little bit more indirection through _multi-level translation_ (implementing page tables in effect as multi-level tree data structures) for the benefit of saving space and avoiding the possibility that a page table overwhelms an entire system's memory.

Generally, paging _translates_ a **virtual address**, what your code sees, to a **physical address**, the place in the hardware ("on-chip") that data ca be found. Virtual addresses are used not only for memory operands (`movq $1, (%rax)` and similar), but to fetch instructions themselves from memory; in effect, every instruction involves at least one memory access, and two accesses for the cases that a memory address is an operand.

Address translation uses a **page number** and a **page offset** for accesses. The page number can be either a _virtual page number_ (VPN), which determines where to focus the lookup within a given page table, or a _physical page number_ (PPN), which determines (when multiplied by the size of a page) the base address of the next-level page in the current translation. The page offset is directly determined by a system's page size: each bit in the page offset can address a byte within a page, i.e., $page offset = $log_{2}($page size$) = 12$$.

**Example**: the following virtual address 0x7fffffffe9a084d4 can be examined for page number and page offset. 

[^1]: On a Unix-like system, particularly Linux.
