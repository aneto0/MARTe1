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
 * 
 */
#if !defined (MENUENTRY_H)
#define MENUENTRY_H

#include "Iterators.h"
#include "Streamable.h"
#include "GCNamedObject.h"
#include "FString.h"
#include "MessageHandler.h"
#include "HttpGroupResource.h"
#include "MenuInterface.h"
OBJECT_DLL(MenuEntry)

/** A class to build a tree o menu pages that operate via Streamable */
class MenuEntry:
    public          MenuInterface,
    public          MessageHandler,
    public          GCNamedObject
{

OBJECT_DLL_STUFF(MenuEntry)


protected:

    /** a specific title, distinct from the object name */
    BString             title;

public:

    /** to be able to choose the labelling policy */
    virtual const char *Title(){
        if (title.Size() == 0)
            return GCNamedObject::Name();
        else
            return title.Buffer();
    }

    virtual void SetTitle(const char *title){
        this->title = title;
    }


    /* Creates a Menu entry with an associated user function */
    MenuEntry(const char *title= ""){
        SetTitle(title);

    }

    /** destroy the object */
    virtual ~MenuEntry(){
    }

    /** set the Title = <titleName>
        Using the above mentioned syntax create a tree of MenuContainer and MenuEntry
        Then individually access the MenuEntry elements to set the user function
    */
    virtual     bool                ObjectLoadSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {

        if (!GCNamedObject::ObjectLoadSetup(info,err)){
            AssertErrorCondition(FatalError,"ObjectLoadSetup:Cannot retrieve name of node");
            return False;
        }

        int size[1] = { 1 };
        int nDim = 1;
        BString title;
        if (!info->ReadArray(&title,CDBTYPE_BString,size,nDim,"Title")){
            AssertErrorCondition(Warning,"ObjectLoadSetup: cannot find the Title entry");
        } else {
            SetTitle(title.Buffer());
        }

        return True;
    }

};



#endif
