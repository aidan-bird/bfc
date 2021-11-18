# Introduction

BF is a minimal esoteric programming language which 8 keywords; all non-keywords are ignored.
A BF program consists of a strip of tape containing cells and a read/write head.
Each keyword performs a single operation on the head, and the program state is stored in the cells.
See the [wiki page](esolangs.org/wiki/Brainfuck) for more details.

# Project Details

This project compiles BF into x86 assembly. 
The program accepts BF code via `stdin`, and then assembly code is printed to `stdout`.
Assemble the output using `nasm`, and then link using `ld` to get an executable.
The helper script `compile-bf.sh` is used to automate the compilation process.

# Usage

```
make
./compile-bf.sh BF_FILE
./BF_FILE.out
# or
./bfc < BF_FILE >> ASM_FILE
nasm -f elf64 ASM_FILE
ld -o ./BF_FILE.out ASM_FILE.o
./BF_FILE.out
```

