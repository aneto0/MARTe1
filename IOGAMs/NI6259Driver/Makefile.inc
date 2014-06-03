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

CFLAGS+= -I. -O2
CFLAGS+= -I../../MARTe/MARTeSupportLib
CFLAGS+= -I../../BaseLib2/Level0
CFLAGS+= -I../../BaseLib2/Level1
CFLAGS+= -I../../BaseLib2/Level2
CFLAGS+= -I../../BaseLib2/Level3
CFLAGS+= -I../../BaseLib2/Level4
CFLAGS+= -I../../BaseLib2/Level5
CFLAGS+= -I../../BaseLib2/Level6
CFLAGS+= -I../../BaseLib2/LoggerService
LDFLAGS+= -L /usr/lib64 -lpxi6259

#The following line enable compilation of TestDMA and TestPIO examples with BaseLib support (commands below do not include BaseLib in the exec)
#all: $(OBJS)  $(TARGET)/NI6259Drv$(GAMEXT) 
all: $(OBJS)  $(TARGET)/NI6259Drv$(GAMEXT)
	echo "`pwd` - build complete [$(OBJS)] -"

include depends.$(TARGET)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)


