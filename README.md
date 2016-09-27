This repository hold the native runtime for the SLVM compiler infrastructure.
The SLVM compiler infrastructure is a LLVM inspired intermediate representation
made in Pharo initially to be able to compile into Spir-V. With the time, this
intermediate representation was extended to support the Smalltalk object model
and the direct generation of X86 (and X86_64) machine code.

This (experimental) repository holds the required runtime to perform message
dispatching, a memory manager and a garbage.

