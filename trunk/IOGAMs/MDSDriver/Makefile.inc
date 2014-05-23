#******************************************************************************
#      $Log: Makefile.inc,v $
#******************************************************************************

OBJSX=

MAKEDEFAULTDIR=../../MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)


CFLAGS+= -I.
CFLAGS+= -I../
CFLAGS+= -I../

CFLAGS+= -I../../BaseLib2/Level0
CFLAGS+= -I../../BaseLib2/Level1
CFLAGS+= -I../../BaseLib2/Level2
CFLAGS+= -I../../BaseLib2/Level3
CFLAGS+= -I../../BaseLib2/Level4
CFLAGS+= -I../../BaseLib2/Level5
CFLAGS+= -I../../BaseLib2/Level6
CFLAGS+= -I../../BaseLib2/LoggerService
CFLAGS+= -I../../Interfaces/MDSInterface

CFLAGS+= -I../../MARTe/MARTeSupportLib
CFLAGS+= -I/usr/local/mdsplus/include
LIBRARIES += -L/usr/local/mdsplus/lib64
LIBRARIES += -lMdsObjectsCppShr


all: $(OBJS)  \
	$(TARGET)/MDSDriver$(DLLEXT)
	echo  $(OBJS)


include depends.$(TARGET)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)
