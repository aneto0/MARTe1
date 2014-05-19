/* 
 * Copyright 2006 The MathWorks, Inc.
 *
 * File   : lcc_dll_main.c $Revision: 1.1.6.1 $
 *
 * Abstract: This file provides the DLL entry point required by LCC
 *
 */

#include <windows.h>

/* DLL entry point */
BOOL WINAPI LibMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
    return 1; 
}
/* EOF: lcc_dll_main.c */
