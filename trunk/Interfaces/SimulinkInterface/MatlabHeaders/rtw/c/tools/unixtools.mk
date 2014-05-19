# Copyright 1994-2010 The MathWorks, Inc.
#
# File    : unixtools.mk   $Revision: 1.21.4.27 $
# Abstract:
#	Setup Unix tools for GNU make

ARCH := $(shell echo "$(COMPUTER)" | tr '[A-Z]' '[a-z]')
OS:=$(shell uname)

#
# Modify the following macros to reflect the tools you wish to use for
# compiling and linking your code.
#


DEFAULT_OPT_OPTS = -O0
ANSI_OPTS        =
CPP_ANSI_OPTS    = 
CPP              = c++
LD               = $(CC)
SYSLIBS          =
LDFLAGS          =
SHRLIBLDFLAGS    =

# Override based on platform if needed

GCC_WALL_FLAG     :=
GCC_WALL_FLAG_MAX :=
ifeq ($(COMPUTER),GLNX86)
  CC  = gcc
  CPP = g++
  DEFAULT_OPT_OPTS = -O0 -ffloat-store -m32
  LDFLAGS = -m32
  SHRLIBLDFLAGS = -shared -m32 -Wl,--no-undefined -Wl,--version-script,
  # Allow ISO-C functions like fmin to be called
  ifeq ($(TGT_FCN_LIB),ISO_C)
    ANSI_OPTS     = -std=c99 -pedantic -fwrapv -fPIC
    CPP_ANSI_OPTS = -fPIC
  else
    ifeq ($(TGT_FCN_LIB),GNU)
      ANSI_OPTS     = -std=gnu99 -pedantic -fwrapv -fPIC
      CPP_ANSI_OPTS = -fPIC
    else
      ifneq ($(NON_ANSI_TRIG_FCN), 1)
         ANSI_OPTS     = -ansi -pedantic -fwrapv -fPIC
         CPP_ANSI_OPTS = -fPIC
      endif
    endif
  endif
  # These definitions are used by targets that have the WARN_ON_GLNX option
  GCC_WARN_OPTS     := -Wall -W -Wwrite-strings -Winline -Wstrict-prototypes \
		     -Wnested-externs -Wpointer-arith -Wcast-align

  GCC_WARN_OPTS_MAX := $(GCC_WARN_OPTS) -Wcast-qual -Wshadow

  ifeq ($(WARN_ON_GLNX), 1)
    GCC_WALL_FLAG     := $(GCC_WARN_OPTS)
    GCC_WALL_FLAG_MAX := $(GCC_WARN_OPTS_MAX)
  endif
endif

ifeq ($(COMPUTER),GLNXA64)
  CC  = gcc
  CPP = g++
  DEFAULT_OPT_OPTS = -O0
  SHRLIBLDFLAGS = -shared -Wl,--no-undefined -Wl,--version-script,
  # Allow ISO-C functions like fmin to be called
  ifeq ($(TGT_FCN_LIB),ISO_C)
    ANSI_OPTS     = -std=c99 -pedantic -fwrapv -fPIC
    CPP_ANSI_OPTS = -fPIC
  else
    ifeq ($(TGT_FCN_LIB),GNU)
      ANSI_OPTS     = -std=gnu99 -pedantic -fwrapv -fPIC
      CPP_ANSI_OPTS = -fPIC
    else
      ifeq ($(TGT_FCN_LIB),ISO_C++)
        ANSI_OPTS     = -std=c++98 -pedantic -fwrapv -fPIC
        CPP_ANSI_OPTS = -fPIC
      else
        ifneq ($(NON_ANSI_TRIG_FCN), 1)
                 ANSI_OPTS     = -ansi -pedantic -fwrapv -fPIC
                 CPP_ANSI_OPTS = -fPIC
        endif
      endif	
    endif
  endif
  # These definitions are used by targets that have the WARN_ON_GLNX option
  GCC_WARN_OPTS     := -Wall -W -Wwrite-strings -Winline -Wstrict-prototypes \
		     -Wnested-externs -Wpointer-arith -Wcast-align

  GCC_WARN_OPTS_MAX := $(GCC_WARN_OPTS) -Wcast-qual -Wshadow

  ifeq ($(WARN_ON_GLNX), 1)
    GCC_WALL_FLAG     := $(GCC_WARN_OPTS)
    GCC_WALL_FLAG_MAX := $(GCC_WARN_OPTS_MAX)
  endif
endif

ifeq ($(COMPUTER),MACI64)
  DEFAULT_OPT_OPTS = -O0 -ffloat-store
  
  # architecture support x86_64 
  ARCHS = x86_64
  LD_NAMESPACE = -twolevel_namespace
  LD_UNDEFS = -undefined error
  CC  = gcc -arch $(ARCHS)
  CPP = g++ -arch $(ARCHS)
  
  ANSI_OPTS = \
    -fno-common -no-cpp-precomp -fexceptions 

  #instead of using -bundle, use -dynamiclib flag to make the lib dlopen compatible
  SHRLIBLDFLAGS = \
    -dynamiclib \
    -Wl,$(LD_NAMESPACE) $(LD_UNDEFS) -Wl,-exported_symbols_list,
endif

# To create a Quantify (from Rational) build,
# specify
#     QUANTIFY=1
# or
#     QUANTIFY=/path/to/quantify
#
# Note, may also need QUANTIFY_FLAGS=-cachedir=./q_cache
#
ifdef QUANTIFY
  ifeq ($(QUANTIFY),1)
    QUANTIFY_ROOT = /hub/$(ARCH)/apps/quantify
  else
    QUANTIFY_ROOT = $(QUANTIFY)
  endif

  INSTRUMENT_INCLUDES := -I$(QUANTIFY_ROOT)
  INSTRUMENT_LIBS     := $(QUANTIFY_ROOT)/quantify_stubs.a
  LD                  := quantify $(QUANTIFY_FLAGS) $(LD)
  OPT_OPTS            := -g
endif

# To create a Purify (from Rational) build, specify
#   PURIFY=1
#
ifeq ($(PURIFY),1)
  PURIFY_ROOT = /hub/$(ARCH)/apps/purify

  INSTRUMENT_INCLUDES := -I$(PURIFY_ROOT)

  ifeq ($(ARCH), sgi)
    INSTRUMENT_LIBS     := $(PURIFY_ROOT)/purify_stubs_n32.a
  else
    INSTRUMENT_LIBS     := $(PURIFY_ROOT)/purify_stubs.a
  endif

  CC       := purify $(CC)
  OPT_OPTS := -g
endif

# [eof] unixtools.mk
