#!/bin/sh

# Aidan Bird 2021 
#
# compile BF
#

prog="$(basename "$0")"
cmd_usage="usage: $(basename "$0") [Path to BF source file]"
[ $# -lt 1 ] && { echo "$cmd_usage"; exit 1; }
./bfc < "$1" > "$1.asm"
nasm -f elf64 "$1.asm"
ld -o "$1.out" "$1.o"
exit 0
