# Notes for CS 3130: Computer Systems and Organization 2

CS 3130 (Computer Systems and Organization 2, or "CSO2") is an upper-division undergraduate course at the University of Virginia, first offered in Spring 2023. The course is best described as a survey of the following topics:
* Operating systems: kernels; exceptions and signals; threads and processes; virtual memory; synchronization primitives
* Computer architecture: caches including TLBs; pipelining; out-of-order (OOO) processors; side-channels and Spectre/Meltdown
* Computer networks: networking basics, TCP/IP, the OSI model, DNS
* Network security: cryptographic primitives, basic principles (symmetric/asymmetric encryption, etc.), certificates, digital signatures

Where programming features in the class (and sometimes it doesn't, particularly with pipelines and OOO), it's almost exclusively C with a little bit of x86 assembly. All of the code examples here are presented in C as such. Code snippets and blocks are documented where prudent (maybe a bit _too much_ since I am quite verbose), though it's still helpful to know C basics going in since CSO2 itself assumes comfortability with C essentials: types, pointers, structs, stack/heap allocation, and so on.

# Sources and Disclaimers

These notes are synthesized from various readings and lecture materials which were developed by Luther Tychonievich and Charles Reiss.
* [Spring 2023](https://www.cs.virginia.edu/~cr4bd/3130/S2023/) readings and course materials available under the [CC BY-NC-SA 4.0 license.](http://creativecommons.org/licenses/by-nc-sa/4.0/)
* [Fall 2023](https://www.cs.virginia.edu/~cr4bd/3130/F2023/) readings and course materials available under the [CC BY-NC-SA 4.0 license.](http://creativecommons.org/licenses/by-nc-sa/4.0/)

**Neither Professor Tychonievich nor Professor Reiss endorse the materials in this repository.** The materials in this repository are a synthesis based substantially on prior work, but aim to present that work in a fresh way. **Nothing within these materials shall be construed as approval or endorsement from the authors of the works from which these notes are derived.**

# License

Except as noted (particularly for source files like [`multitask.c`](./4-processes/multitask.c)), these materials are released under the [CC BY-NC-SA 4.0 license.](https://creativecommons.org/licenses/by-nc-sa/4.0/)

