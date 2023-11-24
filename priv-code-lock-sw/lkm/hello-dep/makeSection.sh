#!/bin/sh
# First make a hello.ko file
sha256sum hello.ko > mydata
../toolchain/bin/riscv64-unknown-linux-gnu-objcopy --add-section .privCodeLock=mydata \
		--set-section-flags .privCodeLock=readonly,alloc hello.o hello2.o
rm hello.o
mv hello2.o hello.o
make
