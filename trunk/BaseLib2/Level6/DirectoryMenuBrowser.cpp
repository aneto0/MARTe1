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

#include "DirectoryMenuBrowser.h"
#include "MenuEntry.h"
#include "HtmlStream.h"

bool DIRMenuSystemTextMenu(DirectoryMenuBrowser &dir,StreamInterface &in,StreamInterface &out){

    // check for inheritance from a GCReferenceContainer
    // if so implement the menu browsing .
    GCRTemplate<MenuInterface> menuIf(&dir);
    if (!menuIf.IsValid()){
        return False;
    }

    GCRTemplate<GCReferenceContainer> container(menuIf);
    if (!container.IsValid()){
        return False;
    }

    DirectoryEntry *localFile = (DirectoryEntry *)dir.directory->List();
    while(localFile != NULL){
        FString fileName = localFile->Name();
        if(localFile->IsFile()){
            GCReference x = dir.Find(fileName.Buffer());
            if(!x.IsValid()){
                GCRTemplate<MenuEntry>  fileSelect(GCFT_Create); 
                fileSelect->SetObjectName(fileName.Buffer());
                fileSelect->SetTitle(fileName.Buffer());
                fileSelect->SetUp(NULL,NULL,NULL,localFile);
                dir.Insert(fileSelect);
            }
        }else if(localFile->IsDirectory()){
            if(fileName == "." ) { 
                fileName = "_DOT_";
            }else if(fileName == "..") {
                fileName = "_DOT_DOT_";
            }
            GCReference x  = dir.Find(fileName.Buffer());
            if(!x.IsValid()){
                FString rPath = dir.relativePath;
                rPath        += DIRECTORY_SEPARATOR;
                rPath        += localFile->Name();

                GCRTemplate<DirectoryMenuBrowser>   newDir(GCFT_Create);
                newDir->SetObjectName(fileName.Buffer());
                newDir->SetTitle(localFile->Name());
                newDir->SetUp(dir.userAction, dir.userData, rPath.Buffer());
                dir.Insert(newDir);
            }
        }
        localFile = (DirectoryEntry *)localFile->Next();
    }


    {
        // lock the container during this next pghase */
        container->Lock();
        GCReferenceContainer menuContainer;

        int i;
        for (i=0;i<container->Size();i++){
            GCRTemplate<MenuInterface> menuItem =  container->Find(i);
            if (menuItem.IsValid()) {
                menuContainer.Insert(menuItem);
            }
        }

        // now I operate on the private container so no worries
        container->UnLock();

        if (menuContainer.Size() > 0){
            if (dir.action != NULL){
                return dir.action(in,out,dir.userData);
            }

            if (dir.entryAction != NULL){
                dir.entryAction(in,out,dir.userData);
            }

            // empty input
            char buffer[128];
            uint32 size = sizeof(buffer);
            int bias = 0;
            while(1){
                out.Printf(
                "###############################################################################\n"
                "##     % 36s                                  ##\n"
                "###############################################################################\n"
                ,dir.Title());
                out.Printf("% 37s#\n","");
                int max = 8;
                for (int i = bias; i<max+bias;i++){
                    GCRTemplate<MenuInterface> m1;
                    if (i       < menuContainer.Size()) m1 = menuContainer.Find(i);
                    GCRTemplate<MenuInterface> m2;
                    if ((i+max) < menuContainer.Size()) m2 = menuContainer.Find(i+max);
                    char label[32];
                    sprintf(label,"%c",i+'A');
                    if (m1.IsValid()) {
                        if (out.Switch("colour")){
                            out.Printf("%i %i",Red,Black);
                            out.Switch((uint32)0);
                        }
                        out.Printf("%s",label);
                        if (out.Switch("colour")){
                            out.Printf("%i %i",Grey,Black);
                            out.Switch((uint32)0);
                        }
                        
                        GCRTemplate<MenuContainer> isDir(m1);
                        if(isDir.IsValid()){
                            out.Printf(":->% 32s #",m1->Title());
                        }else{
                            out.Printf(":  % 32s #",m1->Title());
                        }
                    }  else out.Printf("% 37s#","");
                    label[0] += max;
                    if (m2.IsValid()) {
                        if (out.Switch("colour")){
                            out.Printf("%i %i",Red,Black);
                            out.Switch((uint32)0);
                        }
                        out.Printf(" %s",label);
                        if (out.Switch("colour")){
                            out.Printf("%i %i",Grey,Black);
                            out.Switch((uint32)0);
                        }

                        GCRTemplate<MenuContainer> isDir(m2);
                        if(isDir.IsValid()){
                            out.Printf(":->% 32s    \n",m2->Title());
                        }else{
                            out.Printf(":  % 32s    \n",m2->Title());
                        }
                    } else out.Printf("   % 32s \n","");
                    out.Printf("% 37s#\n","");
                }
                if(dir.Size() > 2*max){
                    out.Printf("###############################################################################\n");
                    if(     bias == 0)               out.Printf("0: EXIT                         >: MOVE DOWN                                   \n");
                    else if(bias > dir.Size()-2*max) out.Printf("0: EXIT         <: MOVE UP                                                     \n");
                    else                             out.Printf("0: EXIT         <: MOVE UP      >: MOVE DOWN                                   \n");
                    out.Printf("###############################################################################\n");
                }else{
                    out.Printf("###############################################################################\n");
                    out.Printf("0: EXIT                                                                        \n");
                    out.Printf("###############################################################################\n");
                }
                size = sizeof(buffer);
                if (!in.Read(buffer,size)) {
                    SleepMsec(100);
                } else {
                    if (strlen(buffer)>0){
                        
                        char command = toupper(buffer[0]);
                        switch(command){
                        case '0': {
                            if (dir.exitAction == NULL) return True;
                            return dir.exitAction(in,out,dir.userData);
                        }break;
                        case '<':{
                            if(bias > 0 ) bias -= (2*max);
                        }break;
                        case '>':{
                            if(bias < dir.Size() - max) bias += (2*max);
                        }break;
                        default:{
                            int index = command - 'A';
                            if ((index >= 0) && (index < menuContainer.Size())){
                                GCReference  m = menuContainer.Find(index);
                                GCRTemplate<MenuEntry> dirEntry(m); 
                                // If it is a DirectoryEntry
                                if(dirEntry.IsValid()){
                                    DirectoryEntry *file = (DirectoryEntry *)dirEntry->UserData();
                                    FString fileName = dir.relativePath;
                                    fileName += DIRECTORY_SEPARATOR;
                                    fileName += file->Name();
                                    fileName.Seek(0);
                                    return dir.userAction(fileName, out, dir.userData);
                                }else{
                                    GCRTemplate<DirectoryMenuBrowser> dir(m);
                                    if(dir.IsValid()){
                                        dir->TextMenu(in,out);
                                    }
                                }
                            }
                        }break;
                        }
                    }
                }
            }
            return True;
        }
    }

    return False;
}




bool DIRMProcessHttpMessage(
                        DirectoryMenuBrowser &mc,
                        HttpStream &hStream){

    FString path = hStream.unMatchedUrl;
/*
    if (hStream.Switch("InputCommands.PATH")){
        hStream.Seek(0);
        hStream.GetToken(path,"");
    }
*/
    
    FString combinedPath = mc.relativePath;
    if(path.Size() > 0){
        if(      combinedPath == "."){
            combinedPath = path;
        }else if((combinedPath[combinedPath.Size() - 1]) == '/'){
            combinedPath += path;
        }else{
            combinedPath += "/";
            combinedPath += path;
        }
    }

    FString command;
    if (hStream.Switch("InputCommands.ACTION")){
        hStream.Seek(0);
        hStream.GetToken(command,"");
    }

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    //copy to the client
    hStream.WriteReplyHeader(False);

    hStream.Printf(
        "<html><head>"
        "<SPAN STYLE=\"color: #FFFFFF;background-color: #000000;\">"
        "<TITLE>%s</TITLE></head>"
        "<BODY BGCOLOR=\"#000000\" FGCOLOR=\"#FFFFFF\" LINK=\"#FF0000\" VLINK=\"#FF8080\" ALINK=\"#FF8080\" >"
        "<H1>%s</H1>",
        mc.Title(),mc.Title());


    Directory dir(combinedPath.Buffer());

/** LIST ALL THE LINKS FIRST */

    hStream.Printf(
        "<TABLE BORDER=0 CELLSPACING=10>"
        "<FORM>"
        "<TR><TD><H3>\n"
        "<SPAN STYLE=\"color: #FFFFFF;background-color: #000000;\">"
        " LINKS </H3></TD></TR>\n"
        "<TR><TD>\n"
        "<H3><A HREF=\"../\">BACK</A> \n"
        "</H3></TD>\n");

    DirectoryEntry *localFile = (DirectoryEntry *)dir.List();
    int buttonCounter = 0;
    while(localFile != NULL){
        FString fileName = localFile->Name();
        if(localFile->IsDirectory()){
            if(fileName == "." ) { 
            }else if(fileName == "..") {
            }else{
                FString totalDirectoryName = combinedPath;
                if(totalDirectoryName == "."){
                    totalDirectoryName = fileName;
                }else if(totalDirectoryName[totalDirectoryName.Size() - 1] == '/'){
                    totalDirectoryName += fileName;
                }else {
                    totalDirectoryName += "/";
                    totalDirectoryName += fileName;
                }

                hStream.Printf(
                    "<TD><H3><A HREF=\"%s/\">%s</A></H3></TD>\n",
                    fileName.Buffer(),fileName.Buffer());

//                hStream.Printf(
//                    "<TD><H3>\n"
//                    "<BUTTON type=\"submit\" name=\"PATH\" value=\"%s\">%s</BUTTON>\n"
//                    "</H3></TD>\n",
//                    totalDirectoryName.Buffer(), fileName.Buffer());
                buttonCounter++;
                if(buttonCounter == 6){
                    buttonCounter = 0;
                    hStream.Printf("</TR>\n<TR>");
                }
            }
        }
        localFile = (DirectoryEntry *)localFile->Next();
    }

    hStream.Printf("</TR>\n</FORM>\n</TABLE>");
    hStream.Printf(
        "<FORM>\n"
        "<INPUT type=\"hidden\" name=\"PATH\" value=\"%s\">", path.Buffer());
    
    hStream.Printf(
        "<TABLE BORDER=0 CELLSPACING=10>\n"
        "<TR><TD><H3>\n"
        "<FONT COLOR=#FFFFFF>\n"
        " ACTIONS </H3></TD></TR>\n"
        "<TR>\n");

    localFile = (DirectoryEntry *)dir.List();
    buttonCounter = 0;
    while(localFile != NULL){
        FString fileName = localFile->Name();
        if(localFile->IsFile()){
            FString totalDirectoryName = combinedPath;
            if(totalDirectoryName == "."){
                totalDirectoryName = fileName;
            }else if(totalDirectoryName[totalDirectoryName.Size() - 1] == '/'){
                totalDirectoryName += fileName;
            }else {
                totalDirectoryName += "/";
                totalDirectoryName += fileName;
            }
            
            hStream.Printf(
                "<TD><H3>\n"
                "<BUTTON type=\"submit\" name=\"ACTION\" value=\"%s\">%s</BUTTON>\n"
                "</H3></TD>\n",
                totalDirectoryName.Buffer(), fileName.Buffer());
            buttonCounter++;
            if(buttonCounter == 6){
                buttonCounter = 0;
                hStream.Printf("</TR>\n<TR>");
            }
        }
        localFile = (DirectoryEntry *)localFile->Next();
    }

    hStream.Printf("</TR>\n</TABLE>\n</FORM>");

    if (command.Size() > 0){

        HtmlStream hsw(hStream);
        hsw.Switch("fontType");
        hsw.Printf("monospace");
        hsw.Switch((uint32)0);
        command.Seek(0);
        mc.userAction(command,hsw,mc.userData);
    }

    hStream +=
        "</BODY>\n";

    return True;
}


OBJECTLOADREGISTER(DirectoryMenuBrowser,"$Id: DirectoryMenuBrowser.cpp,v 1.3 2008/09/25 08:33:30 fpiccolo Exp $")
