DRIVER = hello.ko
ifneq ($(KERNELRELEASE),)
obj-m += hello.o
else
#CFLAGS := "-I$(PWD)/../toolchain/lib/gcc/riscv64-unknown-linux-gnu/7.2.0/include/ -mno-relax"
PWD := $(shell pwd)
ARCH=riscv
CROSS_COMPILE := riscv64-linux-gnu-
LINUXSRC := $(PWD)/../../work/riscv-linux
all:
	@echo '--Building: KDIR=${LINUXSRC} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} EXTRA_CFLAGS=${EXTRA_CFLAGS}'
	CFLAGS=$(CFLAGS) $(MAKE) -C $(LINUXSRC) ARCH=riscv M=$(PWD) modules
endif

clean: 
	rm -f *.o *.ko
