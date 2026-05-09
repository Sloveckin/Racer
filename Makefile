# Name of the module to build
obj-m += proxy.o

# Path to the kernel build directory
KDIR := /lib/modules/$(shell uname -r)/build
# Current working directory
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
