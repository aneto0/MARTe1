# Copyright 1994-2000 The MathWorks, Inc.
#
# File    : watctools.mak   $Revision: 1.4.2.5 $
# Abstract:
#	Setup Watcom tools for wmake.



CC     = $(WATCOM)\binnt\wcc386
CPP    = $(WATCOM)\binnt\wpp386
LD     = $(WATCOM)\binnt\wlink
LIB    = $(WATCOM)\lib386;$(WATCOM)\lib386\nt
LIBCMD = $(WATCOM)\binnt\wlib
AS     = $(WATCOM)\binnt\wasm

PERL   = $(MATLAB_ROOT)\sys\perl\win32\bin\perl

DEFAULT_OPT_OPTS = -od

WATC_STD_OPTS = -bt=NT -5s -e25 -ei -fpi87 -zp8 -zq -fr=

#[EOF] watctools.mak
