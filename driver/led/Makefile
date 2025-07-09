LINUX_KERNEL_PATH = /home/elf/work/ELF2-linux-source/kernel
CURRENT_PATH := $(shell pwd)

obj-m := myled.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules \
		ARCH=arm64 \
		CROSS_COMPILE=/home/elf/work/ELF2-linux-source/prebuilts/gcc/linux-x86/aarch64/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
clean:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
