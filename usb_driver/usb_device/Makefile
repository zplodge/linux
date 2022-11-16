USBTOSDIODIR = $(PWD)

DRIVER_PATH := $(shell pwd)
ARCH := arm
export KERNELDIR=/lib/modules/$(shell uname -r)/build
export KBUILDDIR=/lib/modules/$(shell uname -r)/build

CFG_WORKQUEUE=y
CFG_TASKLET=n
CFG_KTHREAD=n

MOD_SRC = usb_device

obj-m +=$(MOD_SRC).o
$(MOD_SRC)-objs = \
	usb_driver.o \

ccflags-$(CFG_WORKQUEUE) += -DCONFIG_WORKQUEUE
ccflags-$(CFG_TASKLET) += -DCONFIG_TASKLET
ccflags-$(CFG_KTHREAD) += -DCONFIG_KTHREAD

# ccflags-y += -Wall -Werror

default: all

all: modules

modules:
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

