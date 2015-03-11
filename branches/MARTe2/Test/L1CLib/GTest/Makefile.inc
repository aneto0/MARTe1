#############################################################
#
# Copyright 2015 F4E | European Joint Undertaking for ITER 
#  and the Development of Fusion Energy ('Fusion for Energy')
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
# $Id: Makefile.inc 3 2012-01-15 16:26:07Z aneto $
#
#############################################################

OBJSX=  LinkedListableGTest.x 		
	
MAKEDEFAULTDIR=../../../MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

TESTING_LIBRARY=L1CLib

ifeq ($(TARGET),gco)
	BUILD_DIR=gco
else
	BUILD_DIR=../../../Build/$(TARGET)/$(TESTING_LIBRARY)/$(lastword $(subst /,\ ,$(CURDIR)))
endif

CFLAGS+= -I../ -I../../../Source/L0Portability
CFLAGS+= -I../ -I../../../Source/$(TESTING_LIBRARY) 
CFLAGS+= -I../../../Lib/gtest-1.7.0/include

all: $(OBJS)    \
                $(BUILD_DIR)/MainGTest$(EXEEXT)
	echo  $(OBJS)

include depends.$(TARGET)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)

