obj-m += led_dev.o led_drv.o
all:
	make -C /opt/kernel SUBDIRS=$(PWD) modules
clean:
	make -C /opt/kernel SUBDIRS=$(PWD) clean 
