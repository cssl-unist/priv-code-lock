KPATH := ../work/linux

obj-m := pcl.o

.PHONY: all clean

all:
	make -C $(KPATH) M=$(CURDIR) modules

clean:
	make -C $(KPATH) M=$(CURDIR) clean

