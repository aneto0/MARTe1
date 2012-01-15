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

#include "MenuInterface.h"
#include "HttpInterface.h"
#include "MessageHandler.h"
#include "GCReferenceContainer.h"
#include "ConfigurationDataBase.h"
#include "SXMemory.h"

/**
 * @file
 * A class to build a tree of menu pages that operate via Streamable
 */
#if !defined (MENUCONTAINER_H)
#define MENUCONTAINER_H

class MenuContainer;

extern "C" {
    /** */
    bool  MCProcessHttpMessage(MenuContainer &mc,HttpStream &hStream);

}

OBJECT_DLL(MenuContainer)

class MenuContainer:
    public          GCReferenceContainer,
    public          MenuInterface,
    public          MessageHandler,
    public          HttpInterface
{

OBJECT_DLL_STUFF(MenuContainer)


protected:

    /** a specific title, distinct from the object name */
    BString             title;

public:
    /** */
    friend      bool                MCProcessHttpMessage(
                        MenuContainer &             mc,
                        HttpStream &                hStream);

    /** the main entry point for HttpInterface */
    virtual     bool                ProcessHttpMessage(HttpStream &hStream){
        return MCProcessHttpMessage(*this,hStream);
    }

public:

    /** to be able to choose the labelling policy */
    virtual     const char *        Title()
    {
        if (title.Size() == 0)
            return GCNamedObject::Name();
        else
            return title.Buffer();
    }

    virtual     void                SetTitle(
                        const char *                title)
    {
        this->title = title;
    }


    /* Creates a Menu entry with an associated user function */
                                    MenuContainer(
                        const char *                title       = "")
    {
        SetTitle(title);
    }

    /** destroy the object */
    virtual                         ~MenuContainer()
    {
    }

    /**  See GCReferenceContainer for initialisation syntax
        Additionally set the Title = <titleName>
        Using the above mentioned syntax create a tree of MenuContainer and MenuEntry
        Then individually access the MenuEntry elements to set the user function
    */
    virtual     bool                ObjectLoadSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {
        if (!HttpInterface::ObjectLoadSetup(info,err)){
            AssertErrorCondition(InitialisationError,"ObjectLoadSetup: HttpInterface::ObjectLoadSetup failed");
            return False;
        }

        if (!GCReferenceContainer::ObjectLoadSetup(info,err)){
            AssertErrorCondition(InitialisationError,"ObjectLoadSetup: GCReferenceContainer::ObjectLoadSetup failed");
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

    /* Creates a Menu Tree creating a stream first then a CDB and
        then using ObjectLoadSetup  */
                bool                Init(
                        const char *                    configuration,
                        Streamable *                    err             = NULL)
    {
        SXMemory config((char *)configuration,strlen(configuration));
        ConfigurationDataBase cdb;
        if (!cdb->ReadFromStream(config,err)){
            AssertErrorCondition(ParametersError,"Init: cdb.ReadFromStream failed");
            return False;
        }
        return ObjectLoadSetup(cdb,err);
    }

    /** Allows setting up the user functions and data for a specific SubMenu
        The menu is identified by the path to reach the specific menu item    */
                bool                SetupItem(
                        const char *                    name,
                        MenuSystemAction *              action,
                        MenuSystemAction *              entryAction,
                        MenuSystemAction *              exitAction,
                        void *                          userData)
    {

        GCRTemplate<MenuInterface> gctme;
        gctme = Find(name);
        if (gctme.IsValid()){
            gctme->SetUp( action,entryAction,exitAction,userData);
            return True;
        }
        return False;
    }


};



#endif
