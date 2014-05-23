#******************************************************************************
#
#      MARTe library
#      $Log: Makefile.inc,v $
#      Revision 1.1  2009/10/14 15:50:19  abarb
#      MDSplus Interface first partial version
#
#
#******************************************************************************

OBJSX=

MAKEDEFAULTDIR=../../MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

CFLAGS+= -I. -O0 -g
CFLAGS+= -I../../MARTe/MarteSupportLib
CFLAGS+= -I../../BaseLib2/Level0
CFLAGS+= -I../../BaseLib2/Level1
CFLAGS+= -I../../BaseLib2/Level2
CFLAGS+= -I../../BaseLib2/Level3
CFLAGS+= -I../../BaseLib2/Level4
CFLAGS+= -I../../BaseLib2/Level5
CFLAGS+= -I../../BaseLib2/Level6
CFLAGS+= -I../../BaseLib2/LoggerService
CFLAGS+= -I/usr/local/mdsplus/include
LIBRARIES += -L/usr/local/mdsplus/lib64
LIBRARIES += -L../../BaseLib2/linux
LIBRARIES += -lMdsObjectsCppShr

all: $(OBJS)  \
	$(TARGET)/MDSInterface$(DLLEXT)
	echo  $(OBJS)


include depends.$(TARGET)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)
