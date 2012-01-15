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
 * @brief Enables the browsing of a ConfigurationDataBase
 */
#if !defined (CDB_BROWSER_MENU_H)
#define CDB_BROWSER_MENU_H

#include "MenuEntry.h"
#include "CDBHtmlUtilities.h"

class CDBBrowserMenu;
class ConfigurationDataBase;


extern "C" {
    /** */
    bool CDB2Browse(StreamInterface &in,StreamInterface &out,ConfigurationDataBase &cdbRef,const char *cdbName);

    /** */
    bool CDB2Setup(CDBBrowserMenu &cdbBrowse, const char *cdbName, ConfigurationDataBase &cdbRef);

}

OBJECT_DLL(CDBBrowserMenu)

/** allows browsing a CDB via the MenuSystem */
class CDBBrowserMenu:
    public MenuInterface,
    public HttpInterface,
    public GCNamedObject
{
OBJECT_DLL_STUFF(CDBBrowserMenu)

private:

    friend bool CDB2Setup(CDBBrowserMenu &cdbBrowse, const char *cdbName, ConfigurationDataBase &cdbRef);
//    friend bool CDBBM2HtmlObjectSubView(CDBBrowserMenu *cdbbm,HttpStream &hStream);

    /** the database name */
    FString                 cdbName;

    /** The CDB */
    ConfigurationDataBase   cdb;

protected:
    /** The HTTP entry point */
    virtual     bool                ProcessHttpMessage(HttpStream &hStream){
        bool ret = CDBHUHtmlObjectSubView(cdb,Comment(),hStream, CDBHUHOSUV_Header | CDBHUHOSUV_FullBody);
        hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
        //copy to the client
        hStream.WriteReplyHeader(True);
        return ret;
    }

    /** a specific title, distinct from the object name */
    BString             title;

public:

    /** to be able to choose the labelling policy */
    virtual const char *            Title()
    {
        if (title.Size() == 0){
            return Comment();
        } else {
            return title.Buffer();
        }
    }

    virtual void                    SetTitle(
                const char *        title)
    {
        this->title = title;
    }

    /** */
                                    CDBBrowserMenu()
    {
    };

    /** */
                                    ~CDBBrowserMenu()
    {
    };

    /** Sets the database to browse */
                bool                LinkTo(
            const char *            cdbName,
            ConfigurationDataBase & cdb)
    {
        return CDB2Setup(*this,cdbName,cdb);
    }

    /** */
    virtual     bool                TextMenu(
            StreamInterface &           in,
            StreamInterface &           out){
        return CDB2Browse(in,out,cdb,cdbName.Buffer());
    }

    /** save an object content into a set of configs */
    virtual     bool                ObjectSaveSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){

        GCNamedObject::ObjectSaveSetup(info,err);
        return HttpInterface::ObjectSaveSetup(info,err);
    }

    /**  initialise an object from a set of configs */
    virtual     bool                ObjectLoadSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){

        GCNamedObject::ObjectLoadSetup(info,err);
        return HttpInterface::ObjectLoadSetup(info,err);
    }

};


#endif
