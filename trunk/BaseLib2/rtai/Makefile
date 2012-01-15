#############################################################
#
# Copyright 2011 EFDA | European Fusion Development Agreement
#
# Licensed under the EUPL, Version 1.1 or - as soon they 
# will be approved by the European Commission - subsequent  
# versions of the EUPL (the "Licence"); 
# You may not use this work except in compliance with the 
# Licence. 
# You may obtain a copy of the Licence at: 
#  
# http://ec.europa.eu/idabc/eupl
#
# Unless required by applicable law or agreed to in 
# writing, software distributed under the Licence is 
# distributed on an "AS IS" basis, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either 
# express or implied. 
# See the Licence for the specific language governing 
# permissions and limitations under the Licence. 
#
# $Id$
#
##############################################################

TARGET = RTAIBaseLib2

EXTRA_CFLAGS = -DEXPORT_SYMTAB -I/usr/realtime/include
CURRENT = $(shell uname -r)
KDIR = /lib/modules/$(CURRENT)/build
KPWD = $(shell pwd)
DEST = /lib/modules/$(CURRENT)/kernel/$(MDIR)

$(LDFLAGS_NOKVERSION) = -T $(PWD)/../../OSFiles/rtai/C++Sup/global_obj_link.ld

obj-m	:= $(TARGET).o

$(TARGET)-objs := RTAIBaseLib2_mod.o ../Level0/rtai/BaseLib0S.a ../../OSFiles/rtai/C++Sup/global_obj_support.o ../Level1/rtai/BaseLib1S.a ../Level2/rtai/BaseLib2S.a ../Level3/rtai/BaseLib3S.a ../Level4/rtai/BaseLib4S.a ../Level5/rtai/BaseLib5S.a ../Level6/rtai/BaseLib6S.a ../LoggerService/rtai/LoggerService.a

default:
	./makeRTAIBaseLibModCode
	make -C $(KDIR) SUBDIRS=$(KPWD) modules
	
clean:
	-rm -f *.o *.a *.ko .*.cmd .*.flags *.mod.c *.s
