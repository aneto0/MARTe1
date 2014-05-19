# File : rtw/c/src/ext_mode/common/makefile_gnu.mk
#
# Copyright 2007-2008 The MathWorks, Inc.

# $Revision: 1.1.6.1 $

PRODUCT                  := simulink

.PHONY: all
all :

TARGETS = 

ONCE_TARGETS = ext_share.h

once : $(ONCE_TARGETS)

ext_share.h : ../../../../../src/simulink/include/ext_share.h
	rm -f $@
	cp $< $@

cleanonce :
	rm -f $(ONCE_TARGETS)
