#*******************************************************************************
#      $Log: Makefile.inc,v $
#      Revision 1.4  2008/05/08 17:44:35  fpiccolo
#      Moved dependency generation into Makefile.inc
#      Minor bug corrections
#
#      Revision 1.3  2008/05/08 09:56:02  fpiccolo
#      *** empty log message ***
#
#      Revision 1.2  2007/12/19 18:11:25  abarb
#      To be completed
#
#      Revision 1.1  2007/12/19 17:33:03  abarb
#      first working version for VxWorks
#
#*******************************************************************************/

#The following compilation variable will include all NI objects to be linked to our gam 
NI_LIBRARIES =

MAKEDEFAULTDIR=../../MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

CFLAGS+= -I.
CFLAGS+= -I..
CFLAGS+= -I../../MARTe/MARTeSupportLib
CFLAGS+= -I../../BaseLib2/Level0
CFLAGS+= -I../../BaseLib2/Level1
CFLAGS+= -I../../BaseLib2/Level2
CFLAGS+= -I../../BaseLib2/Level3
CFLAGS+= -I../../BaseLib2/Level4
CFLAGS+= -I../../BaseLib2/Level5
CFLAGS+= -I../../BaseLib2/Level6
CFLAGS+= -I../../BaseLib2/LoggerService
CFLAGS+= -I/opt/codac-3.1/epics/base/include
CFLAGS+= -I/opt/codac-3.1/epics/base/include/os/Linux

all: $(OBJS) $(TARGET)/EPICSDrv$(GAMEXT)

include depends.$(TARGET)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)


