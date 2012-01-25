#******************************************************************************
#      RTAIConsole
#      $Id: Makefile.inc,v 1.1 2008/10/27 12:06:36 rvitelli Exp $
#******************************************************************************

OBJSX=
SPB    = api.x module.x impl/Console.x

MAKEDEFAULTDIR=../MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

all: $(OBJS) $(SUBPROJ) 
	echo  $(OBJS)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)

