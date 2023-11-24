# obj-m specifie we're a kernel module.
ifneq ($(KERNELRELEASE),)
obj-m += greeter.o
else
EXTRA_CFLAGS += -I/home/seon/priv-code-lock/freedom-u-sdk/toolchain/lib/gcc/riscv64-unknown-linux-gnu/7.2.0/include/
PWD := $(shell pwd)
CORSS_COMPILE := /home/seon/freedom-u-sdk/toolchain/bin/riscv-unknown-linux-gnu-
LINUXSRC := $(PWD)/../../work/riscv-linux
all:
	@echo '--Building: KDIR=${LINUXSRC} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} EXTRA_CFLAGS=${EXTRA_CFLAGS}'
	$(MAKE) -C $(LINUXSRC) ARCH=riscv M=$(PWD) modules
endif

