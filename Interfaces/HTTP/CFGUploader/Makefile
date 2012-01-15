TARGET1 = RTAIATCAadcDrvGAM
TARGET2 = RTAIATCAdriver_test

EXTRA_CFLAGS = -DEXPORT_SYMTAB -I/usr/realtime/include -D_USES_MAIN_
CURRENT = $(shell uname -r)
KDIR = /lib/modules/$(CURRENT)/build
KPWD = $(shell pwd)
DEST = /lib/modules/$(CURRENT)/kernel/$(MDIR)

$(LDFLAGS_NOKVERSION) = -T $(PWD)/../../../../OSFiles/rtai/C++Sup/global_obj_link.ld

obj-m	:= $(TARGET1).o $(TARGET2).o

$(TARGET1)-objs := ../../../../OSFiles/rtai/C++Sup/global_obj_support.o ATCAadcDrv.o
$(TARGET2)-objs := ../../../../OSFiles/rtai/C++Sup/global_obj_support.o ../../../../BaseLib2/Level0/RTAILoader.o driver_test.o

default:
	make -C $(KDIR) SUBDIRS=$(KPWD) modules
	rm -f ../../../../BaseLib2/Level0/RTAILoader.o
	
clean:
	-rm -f *.o *.a *.ko .*.cmd .*.flags *.mod.c *.s
