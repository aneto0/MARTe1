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
#############################################################
OBJSX=  
MAKEDEFAULTDIR=../../../MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

CFLAGS += -I.
CFLAGS += -I../../../BaseLib2/Level0
CFLAGS += -I../../../BaseLib2/Level1
CFLAGS += -I../../../BaseLib2/Level2
CFLAGS += -I../../../BaseLib2/Level3
CFLAGS += -I../../../BaseLib2/Level4
CFLAGS += -I../../../BaseLib2/Level5
CFLAGS += -I../../../BaseLib2/Level6

all: $(OBJS)    \
	$(TARGET)/ThreadExample1$(EXEEXT)\
    $(TARGET)/AtomicExample1$(EXEEXT)\
    $(TARGET)/ThreadExample2$(EXEEXT)\
    $(TARGET)/StaticListHolderExample$(EXEEXT)\
	$(TARGET)/LinkedListableExample$(EXEEXT)\
    $(TARGET)/FStringExample$(EXEEXT)\
	$(TARGET)/LocalInternetAddressExample$(EXEEXT)\
	$(TARGET)/SimpleTCPServer$(EXEEXT)\
	$(TARGET)/SimpleTCPClient$(EXEEXT)\
	$(TARGET)/SimpleFileHandling$(EXEEXT)\
	$(TARGET)/SimpleConsole$(EXEEXT)\
	$(TARGET)/GCReferenceExample1$(EXEEXT)\
	$(TARGET)/GCReferenceExample2$(EXEEXT)\
	$(TARGET)/GCReferenceContainerExample$(EXEEXT)\
	$(TARGET)/CDBExample1$(EXEEXT)\
	$(TARGET)/CDBExample2$(EXEEXT)\
	$(TARGET)/CDBExample3$(EXEEXT)\
	$(TARGET)/CDBExample4$(EXEEXT)\
	$(TARGET)/GODBExample1$(EXEEXT)\
	$(TARGET)/UDPPing$(EXEEXT)\
	$(TARGET)/UDPPong$(EXEEXT)\
	$(TARGET)/BTDExample1$(EXEEXT)
	echo  $(OBJS)

include depends.$(TARGET)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)

