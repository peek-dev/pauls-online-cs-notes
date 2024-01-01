# x86-64 Quick Reference

This resource is intended to be a quick refresher on x86-64 assembly, and assumes you have already encountered it before elsewhere. I intend to add future readings (probably in the CSO1 directory) that will present x86 "start-to-finish" for a reader who is encountering x86 assembly for the first time. 

# Syntax Differences

Recall that x86 is written in either Intel syntax or AT&T syntax. This author prefers AT&T syntax, so assembly snippets are presented as such.

# AT&T Syntax Symbols

* `$` for **immediates** (i.e., not addresses): `$9`, `$0x1f`, etc.
* `%` for **registers**: `%rax`, `%rbx`, `%rsp`, `%r9`, etc.
* `([REGISTER])` to reference the **address contained in** `[REGISTER]`: `(%rsp)`, `(%rbp)`, `(%rax)`, etc.

# Width Specifiers

| Bits	| Bytes	| How to remember	| AT&T suffix	| Register examples	| Instruction examples	|
| ---	| ---	| ---			| ---		| ---	| --- |
| 8	| 1	| **B**yte		| b		| %ah, %al, %r9b	| movl %ah, %bh	|
| 16	| 2	| **W**ord		| w		| %ax, %r9w		| addw %cx, %dx	|
| 32	| 4	| **D**ouble word	| l		| %eax, %r9d		| subl %ebx, %eax	|
| 64	| 8	| **Q**uad word		| q		| %rax, %r9		| cmpq %rax, %r9; movq %rcx, (%rsp)	|


