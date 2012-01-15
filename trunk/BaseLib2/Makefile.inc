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
SPB    = Level0.x Level1.x Level2.x Level3.x \
         Level4.x Level5.x Level6.x LoggerService.x

MAKEDEFAULTDIR=../MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)


CFLAGS+= -I.
CFLAGS+= -ILevel0
CFLAGS+= -ILevel1
CFLAGS+= -ILevel2
CFLAGS+= -ILevel3
CFLAGS+= -ILevel4
CFLAGS+= -ILevel5
CFLAGS+= -ILevel6
CFLAGS+= -ILoggerService

BLCOMPONENTS = \
       Level0/$(TARGET)/BaseLib0S$(LIBEXT) Level1/$(TARGET)/BaseLib1S$(LIBEXT) \
       Level2/$(TARGET)/BaseLib2S$(LIBEXT) Level3/$(TARGET)/BaseLib3S$(LIBEXT) \
       Level4/$(TARGET)/BaseLib4S$(LIBEXT) Level5/$(TARGET)/BaseLib5S$(LIBEXT) \
       Level6/$(TARGET)/BaseLib6S$(LIBEXT)  \
       LoggerService/$(TARGET)/LoggerService$(LIBEXT)

all: $(OBJS) $(SUBPROJ)\
                $(TARGET)/BaseLib2$(DLLEXT)
	echo  $(OBJS)

$(TARGET)/BaseLib2$(DLLEXT) : $(BLCOMPONENTS)

include depends.$(TARGET)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)

