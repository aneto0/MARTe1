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
    Message.x \
    MessageDeliveryRequest.x \
    MessageEnvelope.x \
    MessageHandler.x \
    MessageBroker.x \
    MessageServer.x \
    MessageDispatcher.x \
    MenuContainer.x \
    MenuInterface.x \
    MenuEntry.x \
    StateMachine.x \
    StateMachineState.x \
    StateMachineEvent.x \
    HttpMessageSendResource.x \
    HttpService.x \
    DDBDefinitions.x \
    DDBSignalDescriptor.x \
    DDBInterfaceDescriptor.x \
    DDBInterface.x \
    DDBItem.x \
    DDB.x \
    GAM.x \
    Signal.x\
	ExecuteMenuEntry.x \
	SendMessageMenuEntry.x \
	StartStopMessageHandlerInterface.x

MAKEDEFAULTDIR=../../MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

CFLAGS+= -I.
CFLAGS+= -I../Level0
CFLAGS+= -I../Level1
CFLAGS+= -I../Level2
CFLAGS+= -I../Level3
CFLAGS+= -I../Level4


all: $(OBJS)    \
                $(TARGET)/BaseLib5S$(LIBEXT) 
	echo  $(OBJS)

include depends.$(TARGET)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)

