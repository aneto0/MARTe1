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
 * @brief Directory browsing
 */
#if !defined _DIRECTORY_MENU_BROSWER
#define _DIRECTORY_MENU_BROSWER

#include "System.h"
#include "MenuContainer.h"
#include "Directory.h"

class DirectoryMenuBrowser;

extern "C"{
    bool DIRMenuSystemTextMenu(DirectoryMenuBrowser &dir,StreamInterface &in,StreamInterface &out);
    bool DIRMProcessHttpMessage(DirectoryMenuBrowser &dir, HttpStream &hStream);
}

OBJECT_DLL(DirectoryMenuBrowser)

class DirectoryMenuBrowser:public MenuContainer{

    friend bool DIRMenuSystemTextMenu( DirectoryMenuBrowser &dir, StreamInterface &in,StreamInterface &out);
    friend bool DIRUserDefinedAction(  StreamInterface &in,StreamInterface &out, void *dir);
    friend bool DIRMProcessHttpMessage(DirectoryMenuBrowser &dir, HttpStream &hStream);

private:

    /** Path relative to the starting position */
    FString             relativePath;

    /** Direcotry */
    Directory           *directory;

    /** File Action. The Selected File Name is passed as the 
        StreamInterface 'in' entry of the userAction. The user must
        GetToken to get the file name in the userAction. 
*/ 
    MenuSystemAction    *userAction;

public:

    DirectoryMenuBrowser(){ 
        directory  = NULL;
        userAction = NULL;
    };

    /** Sets the action associated with this menu item */
    void SetUp( 
                    MenuSystemAction *              action,                    
                    void *                          userData, 
                    const char *path = "."
        )
    {
        this->userAction    = action;
        this->userData      = userData;
        if(directory != NULL) delete directory;
        directory = new Directory(path);
        relativePath = path;
    }

    virtual ~DirectoryMenuBrowser(){
        if(directory != NULL) delete directory;
    };

    /** to call the menu */
    virtual bool TextMenu(
                    StreamInterface &               in,
                    StreamInterface &               out)
    {
        return DIRMenuSystemTextMenu(*this,in,out);
    }


    virtual bool ProcessHttpMessage(HttpStream &hStream){
        return DIRMProcessHttpMessage(*this,hStream);
    }

OBJECT_DLL_STUFF(DirectoryMenuBrowser)
};

#endif

