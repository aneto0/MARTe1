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
CC=gcc
DEBUG=-g3
CURRENT=$(shell uname -r)
CFLAGS=-c $(DEBUG) -Wall -D_REENTRANT -I/usr/realtime/include/ -I/usr/src/linux-$(CURRENT)/include -D_USE_CONSOLE_FIFOS_
LDFLAGS=-L/usr/realtime/lib/
SOURCES=api.c api.h fcomm_ipc.c fcomm_start.c hashtable.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=fcomm_start
LDLOADLIBS=-lpthread -llxrt -lm

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LDLOADLIBS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -rf *.o fcomm_start
