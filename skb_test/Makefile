USBTOSDIODIR = $(PWD)

# KDIR ?= /lib/modules/$(shell uname -r)/build
DRIVER_PATH := $(shell pwd)
#export KERNELDIR ?= $(DRIVER_PATH)/../../../../../../out/target/product/T963/obj/KERNEL_OBJ
#export KBUILDDIR ?= $(DRIVER_PATH)/../../../../../../out/target/product/T963/obj/KERNEL_OBJ
#CROSS_COMPILE ?= /opt/gcc-linaro-6.3.1-2017.02-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
ARCH := arm
export KERNELDIR=/lib/modules/$(shell uname -r)/build
export KBUILDDIR=/lib/modules/$(shell uname -r)/build
# include $(KDIR)/.config
# include $(KERNELDIR)/.config


MOD_SRC = st

obj-m +=$(MOD_SRC).o
$(MOD_SRC)-objs = \
	skb_test.o \


ccflags-y := 


# ccflags-y += -Wall -Werror

default: all


all: modules

modules:
#	$(MAKE) -C $(KERNELDIR) CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) O=$(KBUILDDIR) M=$(USBTOSDIODIR) $@
	$(MAKE) -C $(KERNELDIR) O=$(KBUILDDIR) M=$(DRIVER_PATH) $@
ins:
	@sudo insmod $(MOD_SRC).ko

rm:
	@sudo rmmod $(MOD_SRC).ko

rd:
	@sudo dtoverlay dwc2
	@sudo dtoverlay sdio poll_once=off
	#@sudo insmod ../linux/drivers/usb/gadget/libcomposite.ko

clean:
	@rm -f *.o
	@rm -f *.ko
	@rm -f *.mod.c
	@rm -f modules.order
	@rm -f Module.symvers
	@rm -f *.o.rc
	@rm -f $(MOD_SRC).mod

