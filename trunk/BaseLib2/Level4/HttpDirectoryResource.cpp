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

#include "HttpDirectoryResource.h"
#include "HttpUtilities.h"
#include "HtmlStream.h"
#include "File.h"
#include "Directory.h"

OBJECTLOADREGISTER(HttpDirectoryResource,"$Id$")

bool checkExtension(FString &fname,const char *extension){
    if (extension == NULL) return False;
    int exSize = strlen(extension);
    if (exSize > fname.Size()) return False;

    int pos = fname.Size() - exSize;
    const char *p = fname.Buffer() + pos;
    const char *e = extension;

    while ((toupper(p[0]) == toupper(*e)) && (p[0] != 0)){
        p++;
        e++;
    }
    return (p[0] == 0);
}

bool HttpDirectoryResource::FileAction(FString &fileName,HttpStream &hstream){

    File file;
    if (file.OpenRead(fileName.Buffer()) == True){
        FString temp;
        int64 fSize = file.Size();
        temp.Printf("%Li",fSize);
        hstream.SSPrintf("OutputHttpOtions.Content-Length",temp.Buffer());
        if (checkExtension(fileName,".html") || checkExtension(fileName,".htm"))
            hstream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
        else
        if (checkExtension(fileName,".gif"))
            hstream.SSPrintf("OutputHttpOtions.Content-Type","image/gif");
        else
        if (checkExtension(fileName,".jpg"))
            hstream.SSPrintf("OutputHttpOtions.Content-Type","image/jpg");
        else
        if (checkExtension(fileName,".mpg")||checkExtension(fileName,".mpeg"))
            hstream.SSPrintf("OutputHttpOtions.Content-Type","video/mpeg");
        else
        if (checkExtension(fileName,".jnlp"))
            hstream.SSPrintf("OutputHttpOtions.Content-Type","application/x-java-jnlp-file");
        else
        if (checkExtension(fileName,".js"))
            hstream.SSPrintf("OutputHttpOtions.Content-Type","application/x-javascript");
        else hstream.SSPrintf("OutputHttpOtions.Content-Type","binary");

        // start writing directly to client
        hstream.WriteReplyHeader(False);

        // a way to copy from one stream to the other
        file.Seek(0);

        int bSize = 128000;
        if (bSize > (fSize/4))bSize = (fSize/4);
        char *buffer = (char *)malloc(bSize);
        file.CopyTo(hstream,fSize,TTInfiniteWait,buffer,bSize);
        free((void *&)buffer);
        // end of operation
        hstream.BodyCompleted();        
        return True;
    }
    return False;
}


bool HttpDirectoryResource::ProcessHttpMessage(HttpStream &hStream)
{

    const char *address = hStream.unMatchedUrl.Buffer();
    FString fileAddress;
    fileAddress = baseDir.Buffer();
    fileAddress += '/';
    fileAddress += address;

    // check if it is a file to be sent!
    if (FileAction(fileAddress,hStream)) return True;
    // if it is not a file it might be a directory
    char ending = fileAddress[fileAddress.Size()-1];
    if ((ending !='\\') && (ending != '/')){
        fileAddress += '/';
    }
    FString originAddress;
    originAddress.Printf("/%s/",hStream.url.Buffer());
    FString originAddressHttp;    
    HttpEncode(originAddressHttp,originAddress.Buffer());

    //Just try with the complete file name first
    FString DirectoryIndex;
    DirectoryIndex = fileAddress;
    DirectoryIndex += startHtml;
    if (FileAction(DirectoryIndex,hStream)) return True;
    
    // try with index.html
    DirectoryIndex = fileAddress;
    DirectoryIndex += startHtml;
    DirectoryIndex += ".html";
    // check if it is a file to be sent!
    if (FileAction(DirectoryIndex,hStream)) return True;
    // try with index.htm
    DirectoryIndex = fileAddress;
    DirectoryIndex += startHtml;
    DirectoryIndex += ".htm";
    // check if it is a file to be sent!
    if (FileAction(DirectoryIndex,hStream)) return True;
    HtmlStream hmStream(hStream);
    {

        Directory dir = Directory(fileAddress.Buffer(), fileFilter.Buffer());        
        DirectoryEntry *d =  (DirectoryEntry *)dir.List();
        if (d != NULL){
            hStream.Printf("<html><head><TITLE>%s</TITLE>"
                           "</head><BODY BGCOLOR=\"#ffffff\">"
                           "<H1>%s</H1>\n",originAddress.Buffer(),originAddress.Buffer());
            hmStream.SSPrintf(HtmlTagStreamMode,"TABLE");
            hmStream.SSPrintf(HtmlTagStreamMode,"TR");
            hmStream.SSPrintf(HtmlTagStreamMode,"TD");
            hmStream.Printf("Name");
            hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
            hmStream.SSPrintf(HtmlTagStreamMode,"TD");
            hmStream.Printf("Size");
            hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
            hmStream.SSPrintf(HtmlTagStreamMode,"TD");
            hmStream.Printf("Date");
            hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
            hmStream.SSPrintf(HtmlTagStreamMode,"/TR");

            while(d != NULL){
                if ((strcmp(d->Name(),"..")!=0) && (strcmp(d->Name(),".")!=0)){
                    char stime[64];
                    time_t time = d->Time();
#ifndef _RTAI                    
                    sprintf(stime,"%s",ctime((time_t *)&time));
#else
                    char t[256];
                    ctime(t,256,(time_t *)&time);
#endif
                    stime[strlen(stime)-1]=0;
                    FString fileNameHttp;
                    HttpEncode(fileNameHttp,d->Name());
                    int len = fileNameHttp.Size();

                    hmStream.SSPrintf(HtmlTagStreamMode,"TR");
                    hmStream.SSPrintf(HtmlTagStreamMode,"TD");

                    if (d->IsDirectory()) {
                        hmStream.SSPrintf(HtmlTagStreamMode,"A HREF=\"%s%s/\" NAME=\"%s/\"",originAddressHttp.Buffer(),fileNameHttp.Buffer(),d->Name());
                        hmStream.SSPrintf(ColourStreamMode,"%i %i",Red,White);
                        hmStream.SSPrintf(FontStyleStreamMode,"bold");
                        hmStream.Printf("%s",d->Name());
                        hmStream.SSPrintf(HtmlTagStreamMode,"/A");

                    } else {

                        hmStream.SSPrintf(HtmlTagStreamMode,"A HREF=\"%s%s/\" NAME=\"%s\"",originAddressHttp.Buffer(),fileNameHttp.Buffer(),d->Name());
                        hmStream.SSPrintf(ColourStreamMode,"%i %i",Blue,White);
                        hmStream.Printf("%s",d->Name());
                        hmStream.SSPrintf(HtmlTagStreamMode,"/A");
                    }

                    hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
                    hmStream.SSPrintf(HtmlTagStreamMode,"TD");
                    if (!d->IsDirectory()){
                        hmStream.Printf("%i",d->Size());
                    }
                    hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
                    hmStream.SSPrintf(HtmlTagStreamMode,"TD");
                    hmStream.Printf("%s\n",stime);

                    hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
                    hmStream.SSPrintf(HtmlTagStreamMode,"/TR");
                }
                d= (DirectoryEntry *)d->Next();
            }
            hmStream.SSPrintf(HtmlTagStreamMode,"/TABLE");
            hmStream.SSPrintf(HtmlTagStreamMode,"/BODY");

        } else {
            hStream.Printf("<html><head><TITLE>DIRECTORY %s NOT FOUND</TITLE></head><H1>DIRECTORY %s NOT FOUND</H1></BODY>\n",address,address);
        }
    }
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    //copy to the client
    hStream.WriteReplyHeader(True);

    return True;
}




