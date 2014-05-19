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
#******************************************************************************
#
#      MARTe library
#      $Log: Makefile.inc,v $
#      Revision 1.1  2009/10/14 15:50:19  abarb
#      MDSplus Interface first partial version
#
#
#******************************************************************************

OBJSX= MarteTest0.x
OBJSX += MarteTest0_data.x
OBJSX += rtGetInf.x
OBJSX += rtGetNaN.x
OBJSX += rt_nonfinite.x

MAKEDEFAULTDIR=../../MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

CFLAGS+= -I. -O0 -DRT
CFLAGS+= -I../../MARTe/MARTeSupportLib
CFLAGS+= -I../../BaseLib2/Level0
CFLAGS+= -I../../BaseLib2/Level1
CFLAGS+= -I../../BaseLib2/Level2
CFLAGS+= -I../../BaseLib2/Level3
CFLAGS+= -I../../BaseLib2/Level4
CFLAGS+= -I../../BaseLib2/Level5
CFLAGS+= -I../../BaseLib2/Level6
CFLAGS+= -I../../BaseLib2/LoggerService
CFLAGS+= -I../../Interfaces/SimulinkInterface/MarteTest0_grt_rtw
CFLAGS+= -I../../Interfaces/SimulinkInterface/MatlabHeaders/extern/include
CFLAGS+= -I../../Interfaces/SimulinkInterface/MatlabHeaders/simulink/include
CFLAGS+= -I../../Interfaces/SimulinkInterface/MatlabHeaders/rtw/c/src
LIBRARIES += -L../../../BaseLib2/linux

all: $(OBJS)  \
	$(TARGET)/MarteTestGAM$(DLLEXT)
	echo  $(OBJS)


include depends.$(TARGET)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)
