/*
 * Copyright 2011 EFDA | European Fusion Development Agreement
 *
 * Licensed under the EUPL, Version 1.1 or - as soon they 
   will be approved by the European Commission - subsequent  
   versions of the EUPL (the "Licence"); 
 * You may not use this work except in compliance with the 
   Licence. 
 * You may obtain a copy of the Licence at: 
 *  
 * http://ec.europa.eu/idabc/eupl
 *
 * Unless required by applicable law or agreed to in 
   writing, software distributed under the Licence is 
   distributed on an "AS IS" basis, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either 
   express or implied. 
 * See the Licence for the specific language governing 
   permissions and limitations under the Licence. 
 *
 * $Id$
 *
**/

/**
 * @file
 * Implements all the required functions to load and unload libraries
 */
#ifndef LOADABLE_LIBRARY_H
#define LOADABLE_LIBRARY_H

#include "System.h"

#if defined _VXWORKS
/**  */
typedef int (* OpenModuleFN)();
/**  */
typedef int (* CloseModuleFN)();
/**  */
typedef char *(*ModuleVersionFN)();
#endif

class LoadableLibrary{
private:
#if defined(_CINT)
#elif defined(_OS2)
    ///
    HMODULE module;
#elif (defined (_WIN32) || defined(_RSXNT))
    ///
    HINSTANCE module;
#elif (defined(_LINUX) || defined(_SOLARIS) || defined(_MACOSX))
    void * module;
#elif defined(_RTAI)
    //RTAI requires no init
#elif defined _VXWORKS
    ///
    char *moduleName;

    /** Returns the entry point of the module function searched by name.
        @param fname The name of the function.
     */
    void *FindFunction(const char *fname){
        if (moduleName == NULL) return NULL;

        int size = 512;
        char *fullName = (char *)malloc(size);
        size--;
        strncpy(fullName,moduleName,size);
        size-= strlen(moduleName);
        strncat(fullName,"_",size);
        size--;
        strncat(fullName,fname,size);

        char *value;
        extern SYMTAB_ID sysSymTbl;
        SYM_TYPE type;
        STATUS ret = symFindByName(sysSymTbl,fullName,&value,&type);

        free((void *&)fullName);

        if (ret != OK) return NULL;
        return (void *)value;
    }
#else
#error not supported
#endif
public:
    /** */
    LoadableLibrary(){
#if defined(_OS2) || defined (_WIN32) || defined(_RSXNT)
        module = 0;
#elif defined _VXWORKS
        moduleName = NULL;
#elif (defined(_LINUX) || defined(_SOLARIS) || defined(_MACOSX))
        module = NULL;
#else
#endif
    }

    /** */
    ~LoadableLibrary(){
// Do not unload the DLL from memory.
//        Close();
    }

    /** The function uses OS functions to load a dynamic loadable library.
        The VxWorks implementation however, does not provide a reliable method
        to unload a module so this function is implemented in a different way.
        For VxWorks all the modules are assumed to be linked together and loaded
        in memory. So this function just sets the moduleName variable allowing
        the methods of this class to operate. Moreover, if the library implements
        the functions OpenModule, CloseModule and ModuleVersion, the open, close,
        and ver methods will be made available and the method open is called.
    */
    bool Open(const char *dllName){
#if defined(_OS2)
        if (dllName == NULL) return False;
        if (module != 0) Close();

        char buffer[256];
        APIRET  rc = DosLoadModule(buffer,255, dllName, &module);
        return (rc == 0);
#elif (defined(_LINUX) || defined(_SOLARIS) || defined(_MACOSX))
        if (module != 0) Close();

        module = dlopen(dllName, RTLD_NOW|RTLD_GLOBAL);
        if (module==NULL) return False;
        return True;
#elif (defined (_WIN32) || defined(_RSXNT))
        if (dllName == NULL) return False;
        if (module != 0) Close();

        module = LoadLibrary(dllName);
        return (module != 0);        
#elif defined(_RTAI)        
        if (dllName == NULL) return False;
        //No open in RTAI
#elif defined (_VXWORKS)
        if (dllName == NULL) return False;
        if (moduleName != NULL) free((void *&)moduleName);
#if (defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500))
        moduleName = strdup(dllName);
#elif defined _VX68K
        moduleName = (char *)malloc(strlen(dllName) +  2);
        strcpy(moduleName, "_");
        strcat(moduleName,dllName);
#endif
        OpenModuleFN    open  = (OpenModuleFN)   FindFunction("OpenModule");
        CloseModuleFN   close = (CloseModuleFN)  FindFunction("CloseModule");
        ModuleVersionFN ver   = (ModuleVersionFN)FindFunction("ModuleVersion");

        if ((open == NULL)||(close == NULL) || (ver == NULL)) return False;
        int ret = open();

        return (ret == 0);
#else
        return False;
#endif
    }

    /** The function uses OS functions to unload a dynamic loadable library.
        As for the Open method, the VxWorks implementation differs because
        dynamic linking is not supported. In this case the method close is
        called if provided.
    */
    void Close(){
#if defined(_OS2)
        if (module == 0) return ;

        APIRET rc = DosFreeModule(module);
        return (rc == 0);
#elif (defined(_LINUX) || defined(_SOLARIS) || defined(_MACOSX))
        if (module == 0) return ;

        dlclose(module);
#elif defined(_RTAI) 
        //No close in RTAI
#elif (defined (_WIN32) || defined(_RSXNT))
        if (module == 0) return ;

        FreeLibrary(module);
#elif defined _VXWORKS
        CloseModuleFN   close = (CloseModuleFN)  FindFunction("CloseModule");
        if (moduleName != NULL) free((void *&)moduleName);
        moduleName = NULL;

        if (close == NULL) return;
        close();
#else
#endif
    }

    /** Returns the entry point of the module function specified by name.
    */
    void *Function(const char *name){
        if (name == NULL) return NULL;
#if defined(_OS2)
        if (module == 0) return NULL;
        PFN address;
        APIRET rc = DosQueryProcAddr(module,0,name,&address);
        if (rc == 0) return address;
        return NULL;
#elif (defined(_LINUX) || defined(_SOLARIS) || defined(_MACOSX))
        if((module == NULL) ||(name == NULL)) return NULL;
        return dlsym(module, name);
#elif defined(_RTAI) 
        return fcomm_find_function_by_name((char *)name, NULL);
#elif (defined (_WIN32) || defined(_RSXNT))
        if (module == 0) return NULL;
        void *address = GetProcAddress(module,name);
        //printf(" %x -> %i\n",module,GetLastError());
        return address;
#elif defined _VXWORKS
        return FindFunction(name);
#else
        return NULL;
#endif
    }

    /** Returns the entry point of the module using the [] syntax.
    */
    void *operator[](const char *name){
        return Function(name);
    }

};


#endif
