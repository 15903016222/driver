ifneq ($(KERNELRELEASE),)
    obj-m += at24c02_drv.o
else
    KERNELDIR=/opt/kernel
all:
	PWD=$(shell pwd)  
	$(MAKE) -C $(KERNELDIR) M=$(PWD)
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions module* Module*
endif

