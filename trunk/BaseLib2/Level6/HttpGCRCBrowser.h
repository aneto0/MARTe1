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
 * A Web object to visualise an object structure contained in
 * GCRefereceContainers
 */
#if !defined (HTTP_GCRC_BROWSER)
#define HTTP_GCRC_BROWSER

#include "GCRTemplate.h"
#include "HttpGroupResource.h"

class HttpGCRCBrowser;

OBJECT_DLL(HttpGCRCBrowser)

/** allows browsing a CDB via the MenuSystem */
class HttpGCRCBrowser:
    public HttpGroupResource
{
OBJECT_DLL_STUFF(HttpGCRCBrowser)

private:

protected:
    /** The HTTP entry point */
    virtual bool                    ProcessHttpMessage(HttpStream &hStream);

    /** a specific title, distinct from the object name */
    BString             title;

    void                            ObjectSubView(
            GCRTemplate<GCReferenceContainer>           current,
            const char *                                relativePath,
            const char *                                absolutePath,
            const char *                                selector,
            int                                         level,
            StreamInterface &                           answer);

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
                                    HttpGCRCBrowser()
    {
    };

    /** */
                                    ~HttpGCRCBrowser()
    {
    };


    /** save an object content into a set of configs */
    virtual     bool                ObjectSaveSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){

        return HttpGroupResource::ObjectSaveSetup(info,err);
    }

    /**  initialise an object from a set of configs
        The parameter Root determine the subtree of GODB
        to broswe. Omitting it means the whole tree
        RootIsHttpRoot can be set to 1 if Root is the
        same as that of the Http service. This will mean
        that security will apply correctly and further
        browsing will work */
    virtual     bool                ObjectLoadSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err);

};


#endif
