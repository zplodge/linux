USBTOSDIODIR = $(PWD)

KDIR ?= /lib/modules/$(shell uname -r)/build

include $(KDIR)/.config


MOD_SRC = gadget

obj-m +=$(MOD_SRC).o
$(MOD_SRC)-objs = \
	usb_gadget.o \


ccflags-y := 


# ccflags-y += -Wall -Werror

default: all


all: modules

modules:
	$(MAKE) -j4 -C $(KDIR) M=$$PWD modules

ins:
	@sudo insmod $(MOD_SRC).ko

insf:
	@sudo insmod $(MOD_SRC).ko dl_fw
rm:
	@sudo rmmod $(MOD_SRC).ko

rd:
	@sudo dtoverlay dwc2
#	@sudo dtoverlay sdio poll_once=off
#	@sudo insmod /usr/lib/modules/$(shell uname -r)/kernel/drivers/usb/gadget/libcomposite.ko

clean:
	@rm -f *.o
	@rm -f *.ko
	@rm -f *.mod
	@rm -f *.mod.c
	@rm -f modules.order
	@rm -f Module.symvers

