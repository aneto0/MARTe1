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

BUILD_DIR=gco

EXEEXT=.ex
DLLEXT=.so
LIBEXT=.a
OBJEXT=.o
ASMEXT=.s
DRVEXT=.drv
GAMEXT=.gam
GCNOEXT=.gcno
GCDAEXT=.gcda

OBJS=   $(OBJSX:%.x=$(BUILD_DIR)/%.o) 
OBJS2=  $(OBJSX2:%.x=$(BUILD_DIR)/%.o) 
OBJS3=  $(OBJSX3:%.x=$(BUILD_DIR)/%.o) 

SUBPROJ = $(SPB:%.x=%.spb)
SUBPROJC = $(SPB:%.x=%.spc)
SUBPROJ2 = $(SPB2:%.x=%.spb)
SUBPROJC += $(SPB2:%.x=%.spc)

COMPILER = g++
CCOMPILER = gcc
MAKE     = make

#-maltivec -mabi=altivec
DEBUG = -g -ggdb
OPTIM = 
LFLAGS = -Wl,--no-as-needed  
CFLAGS = -fPIC -Wall
CPPFLAGS = -fPIC -frtti -fprofile-arcs -ftest-coverage 
CFLAGSPEC= -DARCHITECTURE=x86_gcc -DOPERATING_SYSTEM=Linux -DUSE_PTHREAD -pthread
LIBRARIES =  -lm -lnsl -lpthread -lrt -lncurses -ldl -lgcov
.SUFFIXES:   .c  .cpp  .o .a .exe .ex .ex_ .so .gam

