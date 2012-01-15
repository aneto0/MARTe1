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
 * @brief Browsing of objects
 */
#if !defined (OBJECT_BROWSER_MENU_H)
#define OBJECT_BROWSER_MENU_H

#include "MenuEntry.h"

class ObjectBrowserMenu;


extern "C" {
    /** */
    void OBMSetup(ObjectBrowserMenu &obm,const char *className,const char *name,void *data);

    /** */
    bool OBMBrowse(Streamable &in,Streamable &out,char *data,const char *className,const char *name);
}

/** allows browsing a structure content via MenuSystem. structure must be of a registeered type */
class ObjectBrowserMenu: protected MenuEntry{
    friend void OBMSetup(ObjectBrowserMenu &obm,const char *className,const char *name,void *data);
private:
    /** */
    char *className;
    /** */
    char *name;
    /** */
    void *data;

public:
    /** */
    ObjectBrowserMenu(){
        data        = NULL;
        className   = NULL;
        name        = NULL;
    }
    /** */
    ObjectBrowserMenu(const char *className,const char *name,void *data){
        this->data        = NULL;
        this->className   = NULL;
        this->name        = NULL;
        OBMSetup(*this,className,name,data);
    }

    /** */
    ~ObjectBrowserMenu(){
        OBMSetup(*this,NULL,NULL,NULL);
    }

    /** */
    void Setup(const char *className,const char *name,void *data){
        OBMSetup(*this,className,name,data);
    }


    /** */
    bool TextMenu(Streamable &in,Streamable &out){
        return OBMBrowse(in,out,(char *)data,className,name);
    }


};


#endif
