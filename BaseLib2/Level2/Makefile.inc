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
OBJSX= \
        CStreamBuffering.x Streamable.x \
        FString.x GCNString.x \
        File.x BufferedStream.x StreamAttributes.x\
        ATMSocket.x TCPSocket.x UDPSocket.x \
        Console.x RemoteConsole.x CDBDataTypes.x \
        Serial.x

MAKEDEFAULTDIR=../../MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

CFLAGS+= -I.
CFLAGS+= -I../Level0
CFLAGS+= -I../Level1

all: $(OBJS)    \
                $(TARGET)/BaseLib2S$(LIBEXT) 
	echo  $(OBJS)

include depends.$(TARGET)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)

