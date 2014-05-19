# Copyright 1994-2000 The MathWorks, Inc.
#
# File    : vctools.mak   
# Abstract:
#	Setup Visual tools for nmake.

!ifndef DEBUG_BUILD
DEBUG_BUILD = 0
!endif

!if "$(DEBUG_BUILD)" == "0"
NODEBUG = 1
!endif

!include <ntwin32.mak>

CC     = cl
LD     = link
LIBCMD = lib

# /GX or /EHsc enables exception handling (needed to use C++ standard lib)
!if "$(VISUAL_VER)" == "8.0" || "$(VISUAL_VER)" == "9.0" || "$(VISUAL_VER)" == "10.0" || "$(VISUAL_VER)" == "7.1SDK"
CFLAGS_VERSPECIFIC = /wd4996 /fp:precise
CPPFLAGS_VERSPECIFIC = $(CFLAGS_VERSPECIFIC) /EHsc-
!if "$(VISUAL_VER)" == "8.0"
APPVER = 4.0
!else
# APPVER needs to be 5.0 or higher for MSVC 9.0
APPVER = 5.0
!endif
!else
# The following setting of APPVER to 4.0 is needed so that under MSVC 7.0 we
# produce executables compatible with NT 4.0.  Otherwise, MSVC 7.0's ntwin32.mak
# will use a default APPVER of 5.0.  APPVER is the minimum version of Windows
# required, e.g. 5.0 ==> require Win2000 or later.  Doesn't hurt for other
# versions of MSVC so we always do it.

APPVER = 4.0
CFLAGS_VERSPECIFIC = /Op
CPPFLAGS_VERSPECIFIC = $(CFLAGS_VERSPECIFIC) /GX
!endif

# Setting mex architecture argument depends on computer type
MEX_ARCH = -win32
ML_ARCH = win32
!if "$(COMPUTER)" == "PCWIN64"
MEX_ARCH = -win64
ML_ARCH = win64
!endif

#
# Default optimization options.
#
DEFAULT_OPT_OPTS = -Od

#[eof] vctools.mak
