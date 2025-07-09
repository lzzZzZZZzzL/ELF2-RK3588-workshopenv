LINUX_KERNEL_PATH := /home/elf/work/ELF2-linux-source/kernel
CURRENT_PATH := $(shell pwd)
obj-m := bh1750.o
 
build: kernel_modules
 
kernel_modules:
	$(MAKE) -C $(LINUX_KERNEL_PATH) ARCH=arm64 CROSS_COMPILE=/home/elf/aarch64-buildroot-linux-gnu_sdk-buildroot/bin/aarch64-linux-  M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(LINUX_KERNEL_PATH) ARCH=arm64 CROSS_COMPILE=/home/elf/aarch64-buildroot-linux-gnu_sdk-buildroot/bin/aarch64-linux- M=$(CURRENT_PATH) clean
