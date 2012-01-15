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
 * A web server
 */

#if !defined (HTTP_BASIC_SERVICE)
#define HTTP_BASIC_SERVICE

#include "System.h"
#include "GCNamedObject.h"
#include "FString.h"
#include "MultiClientClass.h"
#include "TCPSocket.h"
#include "GCReferenceContainer.h"
#include "GlobalObjectDataBase.h"


/** information passed to the connection thread */
class HttpBasicServiceConnectionInfo: public MultiClientClassConnectionInfo{
public:
    /** */
    MultiClientClass *  base;

    /** */
    TCPSocket *         socket;

    /** */
    virtual ~HttpBasicServiceConnectionInfo(){
        if (socket != NULL) delete socket;
        socket = NULL;
    }
};

class HttpBasicService;

extern "C" {

    /** */
    MCCExitType HSUserServiceThreadEx(HttpBasicService &hs,MultiClientClassConnectionInfo *info);

    /** */
    bool        HSUserListenerInitialize(HttpBasicService &hs);

    /** */
    void        HSUserListenerTerminate(HttpBasicService &hs);

    /** */
    bool        HSUserListenerThread(HttpBasicService &hs,MultiClientClassConnectionInfo *&info);

    /** */
    bool        HSObjectLoadSetup(HttpBasicService &hs,ConfigurationDataBase &info,StreamInterface *err);


}


OBJECT_DLL(HttpBasicService)

/** a mini web server with plugins and realm based security
see MultiClientClass for the activation method
and HttpGroupResource for the resource installation */
class HttpBasicService: public GCNamedObject, public MultiClientClass{

    friend MCCExitType HSUserServiceThreadEx(HttpBasicService &hs,MultiClientClassConnectionInfo *info);
    friend bool        HSUserListenerInitialize(HttpBasicService &hs);
    friend void        HSUserListenerTerminate(HttpBasicService &hs);
    friend bool        HSUserListenerThread(HttpBasicService &hs,MultiClientClassConnectionInfo *&info);
    friend bool        HSObjectLoadSetup(HttpBasicService &hs,ConfigurationDataBase &info,StreamInterface *err);

private:
OBJECT_DLL_STUFF(HttpBasicService)

    /** The server socket */
    TCPSocket                           server;

    /** The port this server is listening to */
    int32                               port;

    /** verboseLevel = 0 means no diagnostics
        1 enables warnings (default)
        5 shows important informations 10 all information */
    int32                               verboseLevel;

    /** http://server:port server that will provide port allocation and relay of http */
    FString                             httpRelayURL;

    /** Where the web pages are contained.
        It will use the URL to search in the container */
    GCRTemplate<GCReferenceContainer>   webRoot;


public:

    /** Use this to select the port to serve at and the name of the root page */
                void        Setup(
            uint32                              port,
            GCRTemplate<GCReferenceContainer>   webRoot,
            const char *                        httpRelayURL=NULL)
    {
        this->port          = port;
        this->webRoot       = webRoot;
        if (!webRoot.IsValid()){
            this->webRoot = GetGlobalObjectDataBase();
        }
        if (httpRelayURL != NULL) this->httpRelayURL = httpRelayURL;
    }

    /** Use this to select the port to serve at and the name of the root page */
                void        Setup(
            uint32                              port)
    {
        this->port          = port;
        this->webRoot = GetGlobalObjectDataBase();
    }

    /** CDB parameters are "Port= <the port listening to> ,
        VerboseLevel = <value for verboseLevel member>
        Root = an object reference within GlobalObjectDataBase
        */
    virtual     bool        ObjectLoadSetup(
            ConfigurationDataBase & info,
            StreamInterface *       err){

        return HSObjectLoadSetup(*this,info,err);
    }

    /** */
    virtual     bool        ObjectSaveSetup(
            ConfigurationDataBase & info,
            StreamInterface *       err){

        return GCNamedObject::ObjectSaveSetup(info,err);
    }

    /** the constructor */
    HttpBasicService() {
        port = 0;
        verboseLevel = 0;
        httpRelayURL = GetHttpRelayURL();
    }
    
    /**
     * The listener name
     */
    virtual const char *ListenerName(){
        return "HS_USER_LISTENER_THREAD";
    }

    /**
     * The service name
     */
    virtual const char *ServiceName(){
        return "HTTP_BASIC_SERVICE";
    }

protected:
    /** The thread performing the connection activity */
    virtual     MCCExitType UserServiceThreadEx(
            MultiClientClassConnectionInfo *info){
        return HSUserServiceThreadEx(*this,info);        
    }

    /** Initialise the server activity */
    virtual     bool        UserListenerInitialize(){
        return HSUserListenerInitialize(*this);
    }

    /** Terminate the connection */
    virtual     void        UserListenerTerminate(){
        HSUserListenerTerminate(*this);
    }

    /** The thread waiting for new connections */
    virtual     bool        UserListenerThread(
            MultiClientClassConnectionInfo *&info){
        return HSUserListenerThread(*this,info);
    }

};


#endif

