obj-m := mbr.o
mbr-objs := mbr-mod.o pkt_output.o pkt_input.o debug.o geohash.o graph.o list.o mbr_route.o neighbors.o 

KDIR :=/home/wu/Desktop/galileo/802.11p-linux

PWD := $(shell pwd)

EXTRA_CFLAGS += -DCONFIG_DEBUG -DCONFIG_MBR_TABLE


default:
	$(MAKE)	-C $(KDIR) SUBDIRS=$(PWD) modules 
clean:
	rm *.o *.ko
	rm *.mod.c
	rm Module.symvers 
	rm modules.order