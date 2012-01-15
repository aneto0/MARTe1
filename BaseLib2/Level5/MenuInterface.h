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

/** @file */

#if !defined (MENUINTERFACE_H)
#define MENUINTERFACE_H

#include "GCNamedObject.h"
#include "Streamable.h"
#include "Console.h"
#include "MessageEnvelope.h"


/** The prototype for a function to associate to a menu page */
typedef bool (MenuSystemAction )(StreamInterface &in,StreamInterface &out,void *userData);

class MenuInterface;

extern "C" {
    /** The standard menu action */
    bool MenuSystemTextMenu(
        MenuInterface &     ms,
        StreamInterface &   in,
        StreamInterface &   out,
        bool                exitImmediately =   False);

}

/** A template class to build a tree o menu pages that operate via Streamable */
class MenuInterface {

friend bool MenuSystemTextMenu(
        MenuInterface &     ms,
        StreamInterface &   in,
        StreamInterface &   out,
        bool                exitImmediately);


protected:

    /** either action is NULL or this is a menu container */
    MenuSystemAction *  action;

    /** what to do when exiting */
    MenuSystemAction *  exitAction;

    /** what to do before entering */
    MenuSystemAction *  entryAction;

    /** contains user specific information */
    void *              userData;

public:

    void *UserData(){return userData;}

public:

    /* Creates a Menu group */
    MenuInterface()
    {
        action      = NULL;
        userData    = NULL;
        exitAction  = NULL;
        entryAction = NULL;
    }

    /** destroy the object */
    virtual ~MenuInterface()
    {
    }

    /** Sets the action associated with this menu item */
    void SetUp(
                    MenuSystemAction *              action,
                    MenuSystemAction *              entryAction,
                    MenuSystemAction *              exitAction,
                    void *                          userData)
    {
        this->userData      = userData;
        this->action        = action;
        this->entryAction   = entryAction;
        this->exitAction    = exitAction;
    }

    /** to be able to choose the labelling policy */
    virtual const char *Title()=0;

    /** to set the appropriate title */
    virtual void SetTitle(const char *title)=0;

    /** to call the menu */
    virtual bool TextMenu(
                    StreamInterface &               in,
                    StreamInterface &               out)
    {
        return MenuSystemTextMenu(*this,in,out);
    }

    /** Called by a class if it is also a MessageHandler */
            bool ProcessMenuMessage(
                    GCRTemplate<MessageEnvelope>    envelope);

};

#endif
