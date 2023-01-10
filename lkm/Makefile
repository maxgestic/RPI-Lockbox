KERNEL := /home/max/Desktop/rpisource/linux
PWD := $(shell pwd)
obj-m += lock.o

all:
	make ARCH=arm CROSS_COMPILE=$(CROSS) -C $(KERNEL) M=$(PWD) modules
clean:
	make -C $(KERNEL) M=$(PWD) clean