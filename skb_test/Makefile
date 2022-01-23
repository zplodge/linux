# 
# wangc add for sdio to usb driver
#
#

USBTOSDIODIR = $(PWD)

KDIR ?= /lib/modules/$(shell uname -r)/build

include $(KDIR)/.config


MOD_SRC = st

obj-m +=$(MOD_SRC).o
$(MOD_SRC)-objs = \
	skb_test.o \


ccflags-y := 


# ccflags-y += -Wall -Werror

default: all


all: modules

modules:
	$(MAKE) -j4 -C $(KDIR) M=$$PWD modules

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

