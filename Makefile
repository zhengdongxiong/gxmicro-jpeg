
DEBUG = y
DRVNAME = qogir_jpeg

ifeq ($(DEBUG),y)
ccflags-y += -DDEBUG
endif

# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)

$(DRVNAME)-objs += qogir_jpeg_drv.o qogir_v4l2.o qogir_ctrls.o qogir_vb2.o qogir_video.o
obj-m += $(DRVNAME).o

# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else

#KERNELDIR ?= /lib/modules/$(shell uname -r)/build
KERNELDIR ?= /home/gx10014/hub/linux/
PWD := $(shell pwd)

default :
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean :
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

install :
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

endif
