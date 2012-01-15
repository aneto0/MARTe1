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

#include "HttpStream.h"
#include "HttpUtilities.h"
#include "SocketTimer.h"
#include "HRT.h"
#include "HttpRealm.h"
#include "BasicTCPSocket.h"
#include "FString.h"

OBJECTREGISTER(HttpStream,"$Id$")


/** initialise to empty */
HttpStream::HttpStream(Streamable *clientStream){
    operationMode       = HSOMWriteToString;
    httpCommand         = HSHCNone;
    httpVersion         = 1000;
    httpErrorCode       = 200;
    keepAlive           = True;
    this->clientStream  = clientStream;
    bodyCompletedEvent.Create();
    /** unknown information length */
    unreadInput         = -1;
}

/** uses this stream to communicate with client
    before exiting reads header  */
bool HttpStream::ReadHeader(){
    /** unknown information length */
    unreadInput         = -1;
    operationMode       = HSOMWriteToString;
    bodyCompletedEvent.Reset();
    lastUpdateTime      = HRT::HRTCounter();
    unsentReplyBody.SetSize(0);
    FString contentType;

    if (clientStream == NULL) return False;

    FString line;
    // Reads the HTTP command
    if (!clientStream->GetLine(line,False)){
        GCNamedObject::AssertErrorCondition(CommunicationError,"LoadStreamAndReadHeader: failed reading a line from socket");
        return False;
    }
    {
        FString command;
        line.Seek(0);
        line.GetToken(command," \n\t",NULL," \n\t");
        if      (command == "GET" ) httpCommand = HSHCGet;
        else if (command == "PUT" ) httpCommand = HSHCPut;
        else if (command == "POST") httpCommand = HSHCPost;
        else if (command == "HEAD") httpCommand = HSHCHead;
        else if (strncmp(command.Buffer(),"HTTP",4)==0) {
            // in a reply there is no command
            // it starts with HTTP ...
            float fVersion = atof(command.Buffer()+5);
            if (fVersion == 0.0) httpVersion = 1000;
            else                 httpVersion = (int)(fVersion * 1000.0);

            FString errorCode;
            line.GetToken(errorCode," \n\t",NULL," \n\t");
            httpCommand = GenerateReplyCode(atoi(errorCode.Buffer()));
        }
        else {
            GCNamedObject::AssertErrorCondition(CommunicationError,"LoadStreamAndReadHeader: Cannot deal with a %s request (GET/PUT/POST/HEAD only)",command.Buffer());
            return False;
        }
    }

    // extract the uri and build a path based n that
    {
        FString tempUrl;
        url.SetSize(0);
        path.SetSize(0);
        line.GetToken(tempUrl," \n?",NULL," \n?");
        tempUrl.Seek(0);
        FString decoded;
        HttpDecode(decoded,tempUrl);
        decoded.Seek(0);
        bool ok = True;
        FString urlPart;
        char saveTerm;
        ok = decoded.GetToken(urlPart,"\\/",&saveTerm); // removed a \t ??
        if (ok){
            url += urlPart.Buffer();
            path += urlPart.Buffer();
            if (saveTerm != 0) url += saveTerm;
        }
        while(ok){
            urlPart.SetSize(0);
            ok = decoded.GetToken(urlPart,"\\/",&saveTerm);
            if (ok){
//                url += '/';
                url += urlPart.Buffer();
                if (saveTerm != 0) url += saveTerm;
                path += '.';
                path += urlPart.Buffer();
            }
        }
    }
    unMatchedUrl = url;

    // extracts commands
    FString commands;
    line.GetToken(commands," \t");

    cdb->MoveToRoot();
    if (cdb->AddChildAndMove("InputCommands")){
        cdb->CleanUp(CDBAM_SubTreeOnly);

        FString command;
        commands.Seek(0);
        while (commands.GetToken(command,"&")){
            command.Seek(0);
            if (command.Size()>3){
                FString variable;
                FString value;
                if (command.GetToken(variable,"=")){
                    command.GetToken(value,"");
                    FString decodedValue;
                    FString decodedVariable;
                    value.Seek(0);
                    variable.Seek(0);
                    HttpDecode(decodedValue,value);
                    HttpDecode(decodedVariable,variable);
                    decodedValue.Seek(0);
                    cdb.WriteFString(decodedValue,decodedVariable.Buffer());
                }
            }
            command.SetSize(0);
        }
    }

    // if httpCommand is a HSHCReply then the version has already been calculated
    if (HSHCReply > httpCommand) {
        FString version;
        line.GetToken(version," \n\t",NULL," \n\t");
        float fVersion = atof(version.Buffer()+5);
        if (fVersion == 0.0) httpVersion = 1000;
        else                 httpVersion = (int)(fVersion * 1000.0);
    }

    Commit();
    cdb->MoveToRoot();
    if (cdb->AddChildAndMove("OutputHttpOtions")){
        cdb->CleanUp(CDBAM_SubTreeOnly);
    }
    cdb->MoveToRoot();
    if (cdb->AddChildAndMove("InputHttpOtions")){
        cdb->CleanUp(CDBAM_SubTreeOnly);

        bool ok = True;
        while (ok){
            line.SetSize(0);
            if (!clientStream->GetLine(line)){
                GCNamedObject::AssertErrorCondition(CommunicationError,"LoadStreamAndReadHeader: failed reading a line from socket");
                return False;
            }
            // terminate on an empty line
            if (line.Size() == 0) ok = False;
            // parse HTTP Options and add to CDB
            else {
                line.Seek(0);
                FString key;
                FString value;
                line.GetToken(key," \t:");
                line.GetToken(value," \t");
                // any other part separated by spaces add to the token
                // use a space as separator
                if (line.Size() > line.Position()) value.PutC(' ');
                line.GetToken(value,"");
                cdb.WriteFString(value,key.Buffer());
            }
        }

        // now evaluate options
        // first check what policy to follow: if dataSize is available and connection keep-alive,
        // then do not shut the connection and load only the specified size
        keepAlive = (httpVersion >= 1100);
        FString connection;
        cdb.ReadFString(connection,"Connection","",False);
        if (strncasecmp(connection.Buffer(),"keep-alive",100)==0){
            keepAlive = True;
        }
        else if (strncasecmp(connection.Buffer(),"close",100)==0){
            keepAlive = False;
        }

        cdb.ReadInt32(unreadInput,"Content-Length",HTTPNoContentLengthSpecified, False);

        cdb.ReadFString(contentType, "Content-Type", "", False);
       
        cdb->MoveToRoot();
    } else {
        GCNamedObject::AssertErrorCondition(FatalError,"LoadStreamAndReadHeader: Cannot move cdb to InputHttpOtions");
        return False;
    }

    //HTTP 1.1 might require to reply to 100-continue so that the client will continue to send more information
    FString expectStr;
    cdb.ReadFString(expectStr, "Expect", "");
    if(expectStr == "100-continue"){
        clientStream->Printf("HTTP/1.1 100 Continue\r\n");
    } 
 
    BString peer = "";
    BasicTCPSocket  *clientSocket = dynamic_cast<BasicTCPSocket  *>(clientStream);
    if (clientSocket != NULL){
        // this could be very slow I think ip number will suffice
        // clientSocket->Source().HostName(peer);
        clientSocket->Source().DotName(peer);
        if (peer == "127.0.0.1"){
            peer = GetLocalAddress();
        }
    }
    cdb.WriteString(peer.Buffer(),"Peer");       

    
    //If it post read the body
    if(httpCommand == HSHCPost){
        if(contentType.Size()  == 0){
            AssertErrorCondition(CommunicationError,"If using POST the Content-Type MUST be specified");
            return False;
        }
        if(unreadInput < 1){
            AssertErrorCondition(CommunicationError,"If using POST the Content-Length MUST be specified");
            return False;
        }
        
        if(!cdb->Move("InputCommands")){
            if (cdb->AddChildAndMove("InputCommands")){
                cdb->CleanUp(CDBAM_SubTreeOnly);
            }
            else{
                AssertErrorCondition(CommunicationError,"Could not create nor add an InputCommands sections to the CDB");
                return False;
            }
        }
        FString  postContent;
        char     buffer[1024];
        uint32   bufferReadSize = 1024;
        while(unreadInput > 0){
            bufferReadSize = 1024;
            if(!clientStream->Read(buffer, bufferReadSize)){
                AssertErrorCondition(CommunicationError,"LoadStreamAndReadHeader: failed reading from socket in POST section");
                return False;
            }
            FString s;
            s.Write(buffer, bufferReadSize);
            if(bufferReadSize == 0){
                break;
            }
            unreadInput -= bufferReadSize;
            if(bufferReadSize > 0){
                if(!postContent.CompleteWrite(buffer, bufferReadSize)){
                    AssertErrorCondition(CommunicationError,"LoadStreamAndReadHeader: failed updating content buffer in POST section");
                    return False;
                }          
            }
        }
        if(!handlePost(contentType, postContent)){
            AssertErrorCondition(CommunicationError,"LoadStreamAndReadHeader: Error handling the post");
            return False;
        }

        cdb->MoveToRoot();
    }

    httpErrorCode = 200;
    return True;
}

bool HttpStream::CompleteReadOperation(Streamable *s,TimeoutType msecTimeout){

    //This way we can change the falsely undefined content-length when this is called from Write Header    
    if(s == NULL && unreadInput == HTTPNoContentLengthSpecified){
        unreadInput = -1;
    }

    if ((unreadInput != -1) && (clientStream)){
        int64 startCounter = HRT::HRTCounter();
        int64 maxTicks     = startCounter + msecTimeout.HRTTicks();

        int bsize = 1024;
        void *buffer = malloc(bsize);
        uint32 size = 1;

        while ((unreadInput > 0 || unreadInput == HTTPNoContentLengthSpecified) && size > 0){
            if(unreadInput == HTTPNoContentLengthSpecified){
                size = bsize;
            }
            else{
                size = unreadInput;
                if (size > bsize) size = bsize;
            }

            if (!clientStream->Read(buffer,size,msecTimeout)){
                GCNamedObject::AssertErrorCondition(Timeout,"CompleteReadOperation: Possible Timeout on completion");
                return False;
            }
            if (s){
                uint32 size2 = size;
                s->CompleteWrite(buffer,size2,msecTimeout);
            }

            if(unreadInput != HTTPNoContentLengthSpecified){
                unreadInput -= size;
            }
            if (msecTimeout.IsFinite()){
                int64 lastCounter = HRT::HRTCounter();
                int64 ticksLeft = maxTicks - lastCounter;
                if (ticksLeft < 0) {
                    GCNamedObject::AssertErrorCondition(Timeout,"CompleteReadOperation: Timeout on completion");
                    return False;
                }
                msecTimeout.SetTimeOutHRTTicks(ticksLeft);
            }
        }
        
        free(buffer);
    }
    return (unreadInput <= 0);
}


bool HSWriteHeader(
                        HttpStream          &hs,
                        bool                bodyCompleted,
                        HSHttpCommand       command,
                        const char *        url){
    return hs._WriteHeader(bodyCompleted,command,url);
}

bool HttpStream::_WriteHeader(
                        bool                bodyCompleted,
                        HSHttpCommand       command,
                        const char *        url){
    if (clientStream == NULL)  return False;
    
    // complete transaction with remote host
    CompleteReadOperation();

    // if it is a reply get errorCode
    // otherwise mark the httpCommand as none
    bool isReply = False;
    if (IsReplyCode(command,httpErrorCode)){
        isReply = True;
    } else {
        httpCommand = HSHCNone;
    }

    // saves all the pending changes
    StreamConfigurationDataBase::Commit();

    // body already written
    if (operationMode == HSOMCompleted) return True;

    // do it if not one yet. OTW its ok
    if (operationMode != HSOMWriteToClient){
        if (bodyCompleted){
            operationMode = HSOMCompleted;
        } else {
            operationMode = HSOMWriteToClient;
        }
    } else {
        return True;
    }

    // assemble the header
    // deal with a reply
    if (isReply){
        if (!clientStream->Printf(
            "HTTP/%i.%i %i %s\r\n",
            httpVersion / 1000,(httpVersion % 1000)/100,httpErrorCode,GetErrorCodeString(httpErrorCode)))
        {
            GCNamedObject::AssertErrorCondition(CommunicationError,"WriteHeader:write on socket failed\n");
            return False;
        }
    } else
    if (command == HSHCGet){
        if (!clientStream->Printf(
            "GET %s HTTP/%i.%i\r\n",            
            url, httpVersion / 1000,(httpVersion % 1000)/100))
        {
            GCNamedObject::AssertErrorCondition(CommunicationError,"WriteHeader:write on socket failed\n");
            return False;
        }
    } else
    if (command == HSHCPut){
        if (!clientStream->Printf(
            "PUT %s HTTP/%i.%i\r\n",
            url, httpVersion / 1000,(httpVersion % 1000)/100))
        {
            GCNamedObject::AssertErrorCondition(CommunicationError,"WriteHeader:write on socket failed\n");
            return False;
        }
    } else
    if (command == HSHCPost){
        if (!clientStream->Printf(
            "POST %s HTTP/%i.%i\r\n",
            url, httpVersion / 1000,(httpVersion % 1000)/100))
        {
            GCNamedObject::AssertErrorCondition(CommunicationError,"WriteHeader:write on socket failed\n");
            return False;
        }
    } else
    if (command == HSHCHead){
        if (!clientStream->Printf(
            "HEAD %s HTTP/%i.%i\r\n",
            url, httpVersion / 1000,(httpVersion % 1000)/100))
        {
            GCNamedObject::AssertErrorCondition(CommunicationError,"WriteHeader:write on socket failed\n");
            return False;
        }
    } else {
        GCNamedObject::AssertErrorCondition(CommunicationError,"WriteHeader:command code %i unknown \n",command);
        return False;
    }

    Commit();
    cdb->MoveToRoot();
    if (cdb->AddChildAndMove("OutputHttpOtions")){

        // if complete we can provide the body size
        if (bodyCompleted && (httpCommand != HSHCHead)){
            cdb.WriteInt32(unsentReplyBody.Size(),"Content-Length");
        } else
        // HEAD is like GET but no body in the reply!
        if (httpCommand == HSHCHead) {
            cdb.WriteInt32(0,"Content-Length");
        }
        FString connection;
        if(cdb.ReadFString(connection,"Connection","", False)){
            if(strncasecmp(connection.Buffer(),"keep-alive",100)==0){
                keepAlive = True;
            }
            else if(strncasecmp(connection.Buffer(),"close",100)==0){
                keepAlive = False;
            }
        }
        if (keepAlive){
            cdb.WriteString("keep-alive","Connection");
        } else {
            cdb.WriteString("close","Connection");
        }

        // write all options
        int index = 0;
        while (cdb->MoveToChildren(index++)){
            FString value;
            FString key;
            if (cdb->NumberOfChildren() < 0){
                if (cdb->NodeName(key) &&
                    cdb.ReadFString(value,"")){

                    if (!clientStream->Printf(
                        "%s:%s\r\n",key.Buffer(),value.Buffer()))
                    {
                        GCNamedObject::AssertErrorCondition(CommunicationError,"WriteHeader:write key %s on socket failed\n",key.Buffer());
                        return False;
                    }
                }
            }
            cdb->MoveToFather();
        }
        clientStream->Printf("\r\n");

        // return to root
        cdb->MoveToRoot();
    } else {
        GCNamedObject::AssertErrorCondition(FatalError,"WriteHeader: Cannot move cdb to InputHttpOtions");
        return False;
    }

    // send out the body
    uint32 toWrite = unsentReplyBody.Size();
    bool ret = clientStream->CompleteWrite(unsentReplyBody.Buffer(),toWrite);

    // notify completion
    if (bodyCompleted) return BodyCompleted();

    return ret;

};


bool HttpStream::SecurityCheck(GCRTemplate<HttpRealm>realm,uint32 ipNumber){
    // no valid realm !
    if (!realm.IsValid()) return True;

    // get key. on failure exit
    BString authorisationKey;
    if (!cdb.ReadBString(authorisationKey,"InputHttpOtions.Authorization", "", False)){
        return False;
    }

    if (!realm->Validate(authorisationKey.Buffer(),httpCommand,ipNumber)){
        return False;
    }

    return True;
}

/** Flag that the writing of the body has been completed */
bool HttpStream::BodyCompleted(){
    operationMode = HSOMCompleted;

    bodyCompletedEvent.Post();
    return True;
}

/** the sender thread waits for the completion of this activity */
bool HttpStream::WaitForBodyCompleted(TimeoutType msecTimeout){
    return bodyCompletedEvent.Wait(msecTimeout);
}

double HttpStream::IdleTime(){
    int64 time = HRT::HRTCounter();
    int64 dt = time - lastUpdateTime;
    double result = dt;
    result = result * HRT::HRTPeriod();

    return result;
}

/** @name Simple Streams INTERFACE
    @{ */

/** remaps SSRead */
bool HttpStream::SSRead(void* buffer,uint32 &size,TimeoutType msecTimeout){
    if(operationMode == HSOMCompleted){
        return False;
    }
    if(operationMode == HSOMWriteToCDB){
        return StreamConfigurationDataBase::SSRead(buffer,size);
    }
    // no content as declared by clent
    if(unreadInput == 0){
        return False;
    }
    if(clientStream == NULL){
        return False;
    }
    bool ret = clientStream->Read(buffer,size,msecTimeout);
    if(unreadInput > 0){
        unreadInput -= size;
    }
    return ret;

}


/** remaps SSWrite */
bool HttpStream::SSWrite(const void* buffer,uint32 &size,TimeoutType msecTimeout){
    lastUpdateTime = HRT::HRTCounter();

    if (operationMode == HSOMCompleted) {
        return False;
    }
    if (operationMode == HSOMWriteToCDB){
        return StreamConfigurationDataBase::SSWrite(buffer,size);
    }

    // we send a body only in the case of HTTP HEAD
    if (httpCommand == HSHCHead) {
        return True;
    }
    if (operationMode == HSOMWriteToClient){
        if (clientStream == NULL) return False;
        return clientStream->Write(buffer,size,msecTimeout);
    }
    return unsentReplyBody.SSWrite(buffer,size);
}


// RANDOM ACCESS INTERFACE

/** The size of the stream
    Note that we cannot read back the size of the actual client stream */
int64 HttpStream::Size(){
    if (operationMode == HSOMCompleted) return False;
    if (operationMode == HSOMWriteToCDB){
        return StreamConfigurationDataBase::Size();
    }
    return unsentReplyBody.Size();
}

/** Move within the stream
    Note that we cannot move on the actual client stream*/
bool  HttpStream::Seek(int64 pos){
    if (operationMode == HSOMCompleted) return False;
    if (operationMode == HSOMWriteToCDB){
        return StreamConfigurationDataBase::Seek(pos);
    }
    return unsentReplyBody.Seek(pos);
}

/** Position within the stream
    Note that we cannot get position of the actual client stream */
int64 HttpStream::Position(void){
    if (operationMode == HSOMCompleted) return False;
    if (operationMode == HSOMWriteToCDB){
        return StreamConfigurationDataBase::Position();
    }
    return unsentReplyBody.Position();
}

/**  Clip the stream size to a specified point
    Note that we cannot do this on the actual client stream */
bool HttpStream::SetSize(int64 size){
    if (operationMode == HSOMCompleted) return False;
    if (operationMode == HSOMWriteToCDB){
        return StreamConfigurationDataBase::SetSize(size);
    }
    return unsentReplyBody.SetSize(size);
}

/** when on write to client we cannot seek !*/
bool  HttpStream::CanSeek(){
    if (operationMode == HSOMCompleted) return False;
    if (operationMode == HSOMWriteToCDB)
        return StreamConfigurationDataBase::CanSeek();
    if (operationMode == HSOMWriteToClient) return False;
    return True;
}

/** @} */

/** @name Extended Attributes or Multiple Streams INTERFACE
    @{ */

/** how many streams are available in total.
    counts from root all the leaves     */
uint32 HttpStream::NumberOfStreams(){
    return StreamConfigurationDataBase::NumberOfStreams() + 1;
}

/** select the stream to read from. Switching may reset the stream to the start. */
bool HttpStream::Switch(uint32 n){
    Commit();
    if (operationMode == HSOMCompleted) return False;
    if (operationMode == HSOMWriteToClient) return False;
    if (n == 0)  {
        operationMode = HSOMWriteToString;
//        StreamConfigurationDataBase::Commit();
        return True;
    } else
    {
        operationMode = HSOMWriteToCDB;
        return StreamConfigurationDataBase::Switch(n-1);
    }
    return False;
}

/** select the stream to read from. Switching may reset the stream to the start. */
bool HttpStream::Switch(const char *name){
    if (operationMode == HSOMCompleted) return False;
    if (operationMode == HSOMWriteToClient) return False;
    if (StreamConfigurationDataBase::Switch(name)){
        operationMode = HSOMWriteToCDB;
        return True;
    }
    return False;
}

/** which is the selected stream */
uint32 HttpStream::SelectedStream(){
    if (operationMode != HSOMWriteToCDB) return 0;
    // return a value to show that we are operating on cache in read only mode
    return StreamConfigurationDataBase::SelectedStream()+1;
}

/** add a new stream to write to. NB selectedStream will not be up to date.... */
bool HttpStream::AddStream(const char *name){
    if (operationMode == HSOMWriteToClient) return False;
    operationMode = HSOMWriteToCDB;
    return StreamConfigurationDataBase::AddStream(name);
}

/** remove an existing stream . */
bool HttpStream::RemoveStream(const char *name){
    if (operationMode == HSOMCompleted) return False;
    if (operationMode == HSOMWriteToClient) return False;
    return StreamConfigurationDataBase::RemoveStream(name);
}

/**Handles the post request*/
bool HttpStream::handlePost(const FString &contentType, FString &content){
    //Write the raw content
    content.Seek(0);
    FString key = "rawPost";
    if(!cdb.WriteFString(content, key.Buffer())){
        AssertErrorCondition(FatalError, "HttpStream::handlePost: Failed writing the raw post content");
        return False;
    }

    FString line;
    content.Seek(0);
    //Check if it is a "multipart/form-data"
    if(strcasestr(contentType.Buffer(), "multipart/form-data") != NULL){
        if(strcasestr(contentType.Buffer(), "boundary=") == NULL){
            AssertErrorCondition(FatalError, "HttpStream::handlePost: In multipart/form-data a boundary must be specified");
            return False;
        }
        //Store the boundary identifier
        FString parsedBoundary = strcasestr(contentType.Buffer(), "boundary=") + strlen("boundary=");
        //Check if the boundary contains quotes and remove if so
        if(parsedBoundary.Size() > 1){
            if((parsedBoundary[0] == '\"') && parsedBoundary[parsedBoundary.Size() - 1] == '\"'){
                parsedBoundary= parsedBoundary.Buffer() + 1;
                parsedBoundary.SetSize(parsedBoundary.Size() - 1);
            }
        }
        FString boundary = "--";
        boundary += parsedBoundary;
        content.Seek(0);        
        line.SetSize(0);
        //The contents (after the header are stored in this field
        FString value;
        value.SetSize(0);
        //The header is not written in the cdb
        bool headerHandled = False;
        //The cdb name key
        FString name  = "Unknown";
        //If a filename is received
        FString filename;
        filename.SetSize(0);
        while(content.GetLine(line)){
            //The header is handled when an empty line is detected
            headerHandled |= (line.Size() == 0);
            if(line.Size() == 0){
                continue;
            }
            //While the header is not handled (separated from main content by empty line)
            //look for name and filename
            if(!headerHandled){
                const char *temp  = NULL;
                int32       count = 0;
                //read the name (it is in the form name="_NAME_")
                if(strcasestr(line.Buffer(), "name=\"") != NULL){
                    name  = strcasestr(line.Buffer(), "name=\"") + strlen("name=\"");
                    temp  = name.Buffer();
                    while(*(temp++) != '\"'){
                        count++;
                    }
                    name.SetSize(count);
                }
                else if(strcasestr(line.Buffer(), "name=") != NULL){
                    name  = strcasestr(line.Buffer(), "name=") + strlen("name=");
                    temp  = name.Buffer();
                    while(*(temp++) != 0){
                        count++;
                    }
                    name.SetSize(count);
                }
                //Check if the content is a file
                if(strcasestr(line.Buffer(), "filename=\"") != NULL){
                    filename = strcasestr(line.Buffer(), "filename=\"") + strlen("filename=\"");
                    temp  = filename.Buffer();
                    count = 0;
                    while(*(temp++) != '\"'){
                        count++;
                    }
                    filename.SetSize(count);
                }
                if(filename.Size() > 0){
                    FString key = name;
                    key += ":filename";

                    //Write the filename
                    if(!cdb.WriteFString(filename, key.Buffer())){
                        AssertErrorCondition(FatalError, "HttpStream::handlePost: Failed writing the filename");
                        return False;    
                    }

                    //Check the file mime type
                    line.SetSize(0);
                    content.GetLine(line);
                    if(strcasestr(line.Buffer(), "Content-Type: ") != NULL){
                        FString fcType = strcasestr(line.Buffer(), "Content-Type: ") + strlen("Content-Type: ");
                        key.SetSize(0);
                        key = name;
                        key += ":Content-Type";
                        if(!cdb.WriteFString(fcType, key.Buffer())){
                            AssertErrorCondition(FatalError, "HttpStream::handlePost: Failed writing the file content type");
                            return False;
                        }
                    }
                }
            } 
            //parse and store the actual message content
            else{
                if(strcasestr(line.Buffer(), boundary.Buffer()) != NULL){
                    headerHandled = False;
                    if(!cdb.WriteFString(value, name.Buffer())){
                        AssertErrorCondition(FatalError, "HttpStream::handlePost: Failed writing the content");
                        return False;
                    }
                    value.SetSize(0);
                    line.SetSize(0);
                    filename.SetSize(0);
                    name.SetSize(0);
                    continue;
                }
                if(value.Size() != 0){
                    value += "\n";
                }
                value += line;
            }
            line.SetSize(0);
        }
        /*if(filename.Size() > 0){
            cdb->MoveToFather();
        }*/
        return True;
    }
    else if(strcasestr(contentType.Buffer(), "application/x-www-form-urlencoded") != NULL){
        //read the content. Key values encoded as in a GET url
        line.SetSize(0);
        content.GetLine(line);
        FString command;
        line.Seek(0);
        while (line.GetToken(command,"&")){
            command.Seek(0);
            if (command.Size()>3){
                FString variable;
                FString value;
                if (command.GetToken(variable,"=")){
                    command.GetToken(value,";");
                    FString decodedValue;
                    FString decodedVariable;
                    value.Seek(0);
                    variable.Seek(0);
                    HttpDecode(decodedValue,value);
                    HttpDecode(decodedVariable,variable);
                    decodedValue.Seek(0);
                    cdb.WriteFString(decodedValue,decodedVariable.Buffer());
                }
            }
            command.SetSize(0);
        }
        return True;
    }
    AssertErrorCondition(FatalError, "HttpStream::handlePost: Content-type handler for: %s not found", contentType.Buffer());
    return True;
}

int32 HttpStream::NumberOfInputCommands(){
    CDBExtended cdbe = GetCDB();
    if(!cdbe->Move("InputCommands")){
        return 0;
    }
    int32 count = cdbe->NumberOfChildren();
    cdbe->MoveToRoot();
    return count;
}

bool HttpStream::InputCommandName(FString &name, int32 idx){
    CDBExtended cdbe = GetCDB();
    if(!cdbe->Move("InputCommands")){
        return False;
    }
    if(!cdbe->MoveToChildren(idx)){
        cdbe->MoveToRoot();
        return False;
    }
    cdbe->NodeName(name);
    cdbe->MoveToRoot();
    
    return True;
}

bool HttpStream::InputCommandValue(FString &value, int32 idx){
    CDBExtended cdbe = GetCDB();
    if(!cdbe->Move("InputCommands")){
        return False;
    }
    if(!cdbe->MoveToChildren(idx)){
        cdbe->MoveToRoot();
        return False;
    }
    bool ok = cdbe.ReadFString(value, "");
    cdbe->MoveToRoot();
    return ok;
}

bool HttpStream::InputCommandValue(FString &value, const char *name){
    CDBExtended cdbe = GetCDB();
    if(!cdbe->Move("InputCommands")){
        return False;
    }
    bool ok = cdbe.ReadFString(value, name, "");
    cdbe->MoveToRoot();

    return ok;
}
