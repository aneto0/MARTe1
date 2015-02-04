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
# $Id: Makefile.inc 3 2012-01-15 16:26:07Z aneto $
#
#############################################################

OBJSX=  Atomic.x \
        Processor.x 

TBD= SPAtomic.x \
        StreamInterface.x \
        ErrorManagement.x \
        CStream.x \
        Processor.x \
        HRT.x \
        Threads.x  \
        ThreadsDatabase.x  \
        Sleep.x \
        Memory.x\
        SemCore.x \
        InternetAddress.x \
        BasicSocket.x \
        BasicATMSocket.x \
        BasicFile.x \
        BasicConsole.x \
        BString.x \
        ProcessorType.x \
		Timer.x

MAKEDEFAULTDIR=../../MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

all: $(OBJS)    \
                $(BUILD_DIR)/L0Portability$(LIBEXT)
	echo  $(OBJS)

include depends.$(TARGET)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)

