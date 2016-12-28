obj-m := relay.o
relay-objs := relay-mod.o pkt_output.o pkt_input.o debug.o

KDIR :=/home/wu/Desktop/galileo/802.11p-linux

PWD := $(shell pwd)

EXTRA_CFLAGS += -DCONFIG_DEBUG


default:
	$(MAKE)	-C $(KDIR) SUBDIRS=$(PWD) modules 
clean:
	rm *.o *.ko
	rm *.mod.c
	rm Module.symvers 
	rm modules.order