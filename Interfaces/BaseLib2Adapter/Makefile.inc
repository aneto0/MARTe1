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
# $Id: Makefile.inc 100 2013-06-17 16:04:31Z aneto $
#
#############################################################
OBJSX=Adapter.x GAMAdapter.x AdapterMessageHandler.x

MAKEDEFAULTDIR=../../MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)


CFLAGS+= -I.
CFLAGS+= -I../../BaseLib2/Level0
CFLAGS+= -I../../BaseLib2/Level1
CFLAGS+= -I../../BaseLib2/Level2
CFLAGS+= -I../../BaseLib2/Level3
CFLAGS+= -I../../BaseLib2/Level4
CFLAGS+= -I../../BaseLib2/Level5
CFLAGS+= -I../../BaseLib2/Level6
CFLAGS+= -I../../BaseLib2/LoggerService

all: $(OBJS)    \
		$(TARGET)/BaseLib2Adapter$(LIBEXT) \
		$(TARGET)/BaseLib2Adapter$(DLLEXT) \
		$(TARGET)/BaseLib2AdapterTest$(EXEEXT)
	echo  $(OBJS)

include depends.$(TARGET)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)

