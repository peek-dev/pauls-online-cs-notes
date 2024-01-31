# What _is_ the operating system?

CSO1 traditionally places "the operating system" into a "black box" which mysteriously mediates between hardware and software. Much of CSO2 is dedicating to revealing parts of the operating system.

An "operating system" is, well, a _system_ (grouping of components with various functional responsibilities) which _operates_ a computer by executing software, handling events on the processor, providing an environment for the user, and more.

What "the operating system" refers to more precisely could include:
1. Privileged code in privileged memory regions of processes (kernel memory), such as those relating to lists of processes, exception handling, and system calls.
2. Item 1. plus system call _wrappers_ (`open`, `close`, `read`, `write`, etc.) and related headers and libraries
3. Item 1. plus processes that run as `root` (the administrator) on startup as well as the accounts and permissions infrastructure that separates users
4. The components of Item 3. packaged together as a software unit (macOS, Microsoft Windows, Ubuntu Linux, etc.)
5. Item 4. plus bundled applications (a text editor, graphical file manager, web browser, etc.)
6. The "permanent" components of Item 5. which are not usually (or easily) customizable by the end user (device drivers, desktop environments, etc.)

