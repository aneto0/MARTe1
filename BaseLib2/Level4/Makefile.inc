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
	Base64Codec.x \
	MD5.x \
	URL.x \
	MultiClientClass.x \
	HttpStream.x  \
	HttpBasicService.x \
	HttpClient.x \
	HttpUtilities.x \
	HtmlStream.x \
	HttpLocalSecurityBasicRealm.x \
	HttpDigestRealm.x \
	HttpGroupResource.x \
	HttpFramedResource.x \
	HttpClassListResource.x \
	HttpDirectoryResource.x \
	HttpServiceRelayResource.x \
	HttpThreadListResource.x \
	HttpRelay.x

MAKEDEFAULTDIR=../../MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

CFLAGS+= -I.
CFLAGS+= -I../Level0
CFLAGS+= -I../Level1
CFLAGS+= -I../Level2
CFLAGS+= -I../Level3
CFLAGS+= -I../LoggerService

all: $(OBJS)    			                \
                $(TARGET)/BaseLib4S$(LIBEXT)
	echo  $(OBJS)

include depends.$(TARGET)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)

