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

OBJSX=  LinkedListableTest.x \
	BStringTest.x \
	LinkedListHolderTest.x \
	CStreamTest.x          	
		

MAKEDEFAULTDIR=../../MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

TESTING_LIBRARY=L1CLib

ifeq ($(TARGET),gco)
	BUILD_DIR=gco
else
	BUILD_DIR=../../Build/$(TARGET)/$(TESTING_LIBRARY)/Test
endif

CFLAGS+= -I../../Source/L0Portability
CFLAGS+= -I../../Source/$(TESTING_LIBRARY)

all: $(OBJS) \
                $(BUILD_DIR)/$(TESTING_LIBRARY)Test$(LIBEXT)                 
	echo  $(OBJS)

include depends.$(TARGET)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)

