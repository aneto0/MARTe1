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
#define GCRLOADPOINTER

#include "HttpGCRCBrowser.h"
#include "CDBExtended.h"
#include "HtmlStream.h"
#include "GlobalObjectDataBase.h"
#include "CDBHtmlUtilities.h"


OBJECTLOADREGISTER(HttpGCRCBrowser,"$Id: HttpGCRCBrowser.cpp,v 1.9 2011/09/16 15:15:10 aneto Exp $")


bool HttpGCRCBrowser::ObjectLoadSetup(
        ConfigurationDataBase &     info,
        StreamInterface *           err){

    CDBExtended cdbx;

    return HttpGroupResource::ObjectLoadSetup(info,err);
}

void HttpGCRCBrowser::ObjectSubView(
                    // ref to current object. initially is HttpGCRCBrowser
                    GCRTemplate<GCReferenceContainer>   current,
                    // the path relative to the start point / separated
                    const char *                        relativePath,
                    // the absolute start point . separated
                    const char *                        absolutePath,
                    // choses what to show
                    const char *                        selector,
                    int                                 level,
                    StreamInterface &                   answer)
{

    // if not abort
    if (!current.IsValid()) return;

    answer.Printf( "<TABLE  > \n"); //border=\"1\"

    // LIST CONTENT
    int numberOfChildren = current->Size();
    for (int i = 0;i< numberOfChildren;i++){

        // check if GCNO
        GCRTemplate<GCNamedObject> currentGCNO = current->Find(i);
        if (currentGCNO.IsValid()){

            const char *nodeName = currentGCNO->Name();
            const char *className = currentGCNO->ClassName();

            // fullNodeName contains the relativePath/NodeName
            FString fullNodeName;
            if ((relativePath == NULL) || (relativePath[0] == 0)){
                fullNodeName = nodeName;
            } else {
                fullNodeName.Printf("%s/%s",relativePath,nodeName);
            }

            // absoluteFullNodeName is always terminated with /
            FString absoluteFullNodeName;
            absoluteFullNodeName = fullNodeName;
            if (absoluteFullNodeName.Size() > 0){
                if (absoluteFullNodeName[absoluteFullNodeName.Size()-1]!='/'){
                    absoluteFullNodeName += "/";
                }
            } else {
                absoluteFullNodeName = "/";
            }

            FString updatedAbsolutePath;
            updatedAbsolutePath = absolutePath;
            if (updatedAbsolutePath.Size() > 0) updatedAbsolutePath += ".";
            updatedAbsolutePath += nodeName;

            answer.Printf( "<TR>\n");

            // in this TAB all the buttons
            answer.Printf( "<TD>\n");

            /* check potential structure browsing capability */
            bool structureFieldDone = False;
            ObjectRegistryItem *ori =  currentGCNO->Info();
            if (ori != NULL){
                ClassStructure *cs = ori->structure;
                if (cs != NULL){
                    structureFieldDone = True;
                    answer.Printf(
                        "<INPUT type=\"button\" name=\"%s\" title=\"Structure\" value=\"S\" size=1 "
                        "ONCLICK=\"Structure('%s')\">\n"
                        ,nodeName,updatedAbsolutePath.Buffer());
                }
            }
//            if (!structureFieldDone){
//                answer.Printf(
//                    "<TD>\n"
//                    "</TD>\n");
//            }

            // if True it means that this a further level down will be explored
            bool recurse = False;
            GCRTemplate<GCReferenceContainer> currentGCRC = currentGCNO;

            // a container
            if (currentGCRC.IsValid()){
                FString wrappedFullNodeName;
                wrappedFullNodeName.Printf("|%s|",fullNodeName.Buffer());

                // check whether to expand this
                if (strstr(selector,wrappedFullNodeName.Buffer())){
                    FString updatedSelector;
                    updatedSelector = selector;
                    const char *p = strstr(selector,fullNodeName.Buffer());
                    if (p!= NULL){
                        int offset = p - selector;
                        if (offset > 0) offset--;
                        updatedSelector.SetSize(offset);
                        p += fullNodeName.Size();
                        updatedSelector +=  p;
                    }
                    if (updatedSelector.Size() == 0) updatedSelector = "|";

                    answer.Printf(
//                        "<TD>\n"
                        "<INPUT type=\"button\" name=\"%s\" title=\"-\" value=\"-\" size=1 "
                        "ONCLICK=\"Expand('%s')\">\n"
//                        "</TD>\n"
                        ,nodeName,updatedSelector.Buffer());

                    // force expansion to next level
                    recurse = True;

                }
                else { // not expanded
                    FString updatedSelector;
                    if (strlen(selector)>0) {
                        updatedSelector.Printf("%s%s|",selector,fullNodeName.Buffer());
                    } else {
                        updatedSelector.Printf("%s|",fullNodeName.Buffer());
                    }

                    answer.Printf(
//                        "<TD>\n"
                        "<INPUT type=\"button\" name=\"%s\" title=\"+\" value=\"+\" size=1 "
                        "ONCLICK=\"Expand('%s')\">\n"
//                        "</TD>\n"
                        ,nodeName,updatedSelector.Buffer());

//                    answer.Printf(
//                        "<INPUT type=\"button\" name=\"%s\" title=\">\" value=\">\" size=1 "
//                        "ONCLICK=\"Delve('%s')\">\n"
//                        ,nodeName,updatedAbsolutePath.Buffer());
                }

            } else { // not a container
//                answer.Printf(
//                    "<TD>\n"
//                    "</TD>\n"  );
//                answer.Printf(
//                    "<TD>\n"
//                    "</TD>\n"  );
            }

            // end of button group
            answer.Printf( "</TD>\n");

            answer.Printf(
                "<TD>\n"
                "(%s)\n"
                "</TD>\n"
                ,className);

            /** reference to object own page */
            GCRTemplate<HttpInterface> currentHI = currentGCNO;
            if (currentHI.IsValid()){
                answer.Printf(
                    "<TD>%s\n"
                    ,nodeName
                    );
                answer.Printf(
                    "<INPUT type=\"button\" name=\"WWW\" title=\"open %s on side panel\" value=\">\" size=1 "
                    "ONCLICK=\"LoadHttpPage(\'%s\')\">\n"
                    ,absoluteFullNodeName.Buffer()
                    ,absoluteFullNodeName.Buffer()
                    );

                answer.Printf(
                    "<INPUT type=\"button\" name=\"WWW\" title=\"open %s on window\" value=\"W\" size=1 "
                    "ONCLICK=\"OpenHttpPage(\'%s\')\">\n"
                    ,absoluteFullNodeName.Buffer()
                    ,absoluteFullNodeName.Buffer()
                    );

                answer.Printf(
                    "</TD>\n"
                    );
            } else {
                /** reference to CDB page */
                GCRTemplate<CDBVirtual> currentCDB = currentGCNO;
                if (currentCDB.IsValid()){
                    answer.Printf(
                        "<TD>%s\n"
                        ,nodeName
                        );
                    answer.Printf(
                        "<INPUT type=\"button\" name=\"WWW\" title=\"open %s on side panel\" value=\"CDB >\" size=1 "
                        "ONCLICK=\"LoadCDBPage(\'%s\')\">\n"
                        ,absoluteFullNodeName.Buffer()
                        ,absoluteFullNodeName.Buffer()
                        );

                    answer.Printf(
                        "<INPUT type=\"button\" name=\"WWW\" title=\"open %s on window\" value=\"CDB W\" size=1 "
                        "ONCLICK=\"OpenCDBPage(\'%s\')\">\n"
                        ,absoluteFullNodeName.Buffer()
                        ,absoluteFullNodeName.Buffer()
                        );

                    answer.Printf(
                        "</TD>\n"
                        );
                } else {
                    answer.Printf(
                        "<TD>\n"
                        "%s\n"
                        "</TD>\n"
                        ,nodeName);
                }
            }

            /* end of table row */
            answer.Printf( "</TR>\n");

            if (recurse){
                answer.Printf( "</TABLE  > \n");
                answer.Printf( "<TABLE  > \n");
                answer.Printf( "<TR  > \n");
                answer.Printf( "<TD  > \n");
                answer.Printf( "------> \n");
                answer.Printf( "</TD  > \n");
                answer.Printf( "<TD  > \n");
                ObjectSubView(currentGCRC,fullNodeName.Buffer(),updatedAbsolutePath.Buffer(),selector,level+1,answer);
                answer.Printf( "</TD  > \n");
                answer.Printf( "</TABLE  > \n");
                answer.Printf( "<TABLE  > \n");
            }

        } // valid child
    } // end loop on each child
    answer.Printf( "</TABLE>\n");
}

bool HttpGCRCBrowser::ProcessHttpMessage(HttpStream &hStream){

    /** see if a structure view is requested */
    FString mode;
    if(!hStream.InputCommandValue(mode, "mode")){
        hStream.Printf("<html>\n<head><TITLE>%s</TITLE>\n",Comment());
        hStream.Printf("<FRAMESET COLS=\"1,1\">\n");
        hStream.Printf("<FRAME SRC=\"?mode=~MAINVIEW~\" NAME=\"MainView\">\n");
        hStream.Printf("<FRAME SRC=\"?mode=~START~\" NAME=\"StructView\">\n");
        hStream.Printf("</HTML>\n");
    }

    /** see if a structure view is requested */
    if(mode == "~STRUCTVIEW~"){
        // what subtree to open
        FString objectName;

        if(hStream.InputCommandValue(objectName, "objName")){
            hStream.Printf("<html>\n<head><TITLE>%s</TITLE>\n",Comment());
            //Remove the last /
            if(objectName.Size() > 0){
                if(objectName[objectName.Size() - 1] == '/'){
                    objectName.SetSize(objectName.Size() - 1);
                }
            }
            /* find object */
            GCRTemplate<Object>  gcno = Find(objectName.Buffer());
            if (gcno.IsValid()){
                /* structure information available? */
                ObjectRegistryItem *ori = gcno->Info();
                if (ori != NULL){
                    /* first map memory to a MMCDB */
                    ConfigurationDataBase mcdb("MMCDB");
                    // use the Object * pointer to the object and using the objectOffset information find the real start address
                    char *ptr =(char *)gcno.operator->();
                    ptr -= ori->objectOffset;
                    /* this operation performs the mapping */
                    mcdb->WriteStructure(gcno->ClassName(),ptr,objectName.Buffer());

                    /* this library is the same used by CDBBrowser */
                    /* it produce the web page and also allows writing to the memory */
                    bool ret = CDBHUHtmlObjectSubView(mcdb,"Structure-Browser",hStream, CDBHUHOSUV_Header | CDBHUHOSUV_FullBody | CDBHUHOSUV_NoBack);

                    /* only need to complete the http message */
                    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
                    //copy to the client
                    hStream.WriteReplyHeader(True);
                    return ret;
                }
            }
        }
        /* in case of failure */
        hStream.Printf(
            "<html>\n"
            "<head>\n"
            "<TITLE>OBJECT NOT FOUND</TITLE>\n"
            "<BODY BGCOLOR=\"#ffffff\"><H1>%s</H1><UL>\n"
            "OBJECT %s NOT FOUND\n"
            "</BODY>\n"
            "\n"
            ,objectName.Buffer(), objectName.Buffer()
            );

        hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
        //copy to the client
        hStream.WriteReplyHeader(True);

        return True;

    }

    if(mode == "~START~"){
        hStream.Printf("<html>\n<head><TITLE>%s</TITLE>\n",Comment());
        hStream.Printf(
            "</head>\n"
            "<BODY BGCOLOR=\"#ffffff\"><H1>GCRCBrowser</H1><UL>\n"
            "<p>Use the menu on the left to navigate in the object list.</p>\n"
            "</BODY>\n"
            "\n"
            );

        hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
        //copy to the client
        hStream.WriteReplyHeader(True);

        return True;

    }

    /** see if a structure view is requested */
    if(mode == "~CDBVIEW~"){
        // this pointer we use to search for the end of objectname
        char *remainder = hStream.unMatchedUrl.BufferReference()+9;
        // when the end is terminated correctly with 0 then this string will become objectName
        const char *objectNamePtr = hStream.unMatchedUrl.Buffer()+9;

        // what subtree to open
        FString objectName;

        /* separate the object name by placing a 0 on the / if needed */
        /* update unmatched URL */
        while ((remainder[0] !=0) && (remainder[0] != '/')) remainder++;
        if (remainder[0] ==0) {
            objectName = objectNamePtr;
            hStream.unMatchedUrl = "";
        } else {
            remainder[0] = 0;
            remainder++;
            objectName = objectNamePtr;
            hStream.unMatchedUrl = remainder;
        }


        /* find object */
        GCRTemplate<CDBVirtual>  gcno = Find(objectName.Buffer());
        if (gcno.IsValid()){
            ConfigurationDataBase cdb(gcno);

            /* this library is the same used by CDBBrowser */
            /* it produce the web page and also allows writing to the memory */
            FString name = "CDB-Browser: ";
            name += objectName.Buffer();
            bool ret = CDBHUHtmlObjectSubView(cdb,name.Buffer(),hStream, CDBHUHOSUV_Header | CDBHUHOSUV_FullBody | CDBHUHOSUV_NoBack);
            /* only need to complete the http message */
            hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
            //copy to the client
            hStream.WriteReplyHeader(True);
            return ret;
        }

        /* in case of failure */
        hStream.Printf(
            "<html>\n"
            "<head>\n"
            "<TITLE>OBJECT NOT FOUND</TITLE>\n"
            "<BODY BGCOLOR=\"#ffffff\"><H1>%s</H1><UL>\n"
            "OBJECT %s NOT FOUND\n"
            "</BODY>\n"
            "\n"
            ,objectName.Buffer(), objectName.Buffer()
            );

        hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
        //copy to the client
        hStream.WriteReplyHeader(True);

        return True;
    }


    /** see if a structure view is requested */
    if(mode != "~MAINVIEW~"){
        return False;
    }

    // what subtree to open
    FString SubAddress;
    SubAddress = "";
    if (hStream.Switch("InputCommands.SubAddress")){
        hStream.Seek(0);
        hStream.GetToken(SubAddress,"");
        hStream.Switch((uint32)0);
    }
    const char *address = SubAddress.Buffer();

    /** The pointer to the current position */
    GCRTemplate<GCReferenceContainer> current(this);

    // if not abort
    if (!current.IsValid()){
        return False;
    }

    // if an absolutePath is specified switch to the object specified
    if (SubAddress.Size() > 0){
        GCRTemplate<GCReferenceContainer> currentC= current->Find(SubAddress.Buffer());
        current = currentC;
    }
    // if the object specified does not exist (or is not a container) exit
    if (!current.IsValid()) {
        return False;
    }

    // what subtree to open
    FString ViewSelector;
    ViewSelector = "|";
    if (hStream.Switch("InputCommands.ViewSelector")){
        hStream.Seek(0);
        hStream.GetToken(ViewSelector,"");
        hStream.Switch((uint32)0);
    }

    FString title;
    title = Comment();
    if (strlen(address) > 0){
        title += '.';
        title += address;
    }

    /* calculate the back location */
    FString back;
    if ((address != 0) && (strlen(address)>0)){
        back = address;
        int ix = back.Size() -1;
        while((ix > 0) && (address[ix] != '.'))ix--;
        if (ix >= 0) ix--;
        back.SetSize(ix+1);
        if (back.Size()==0) {
            back = '.';
        }
    } else back = "..";

    // now build the page
    {
        HtmlStream hmStream(hStream);

        hStream.Printf(
            "<html>\n"
            "<head>\n"
            "<TITLE>%s</TITLE>\n"
            "<script type=\"text/javascript\">\n"
            "   function Expand(expand){\n"
            "       parent.MainView.location.replace(\"?mode=~MAINVIEW~&ViewSelector=\" + expand + \"&SubAddress=%s\");\n"
            "       return true;\n"
            "   }\n"
            "   function Delve(expand){\n"
            "       parent.MainView.location.replace(\"?mode=~MAINVIEW~&SubAddress=\" + expand);\n"
            "       return true;\n"
            "   }\n"
            "   function Structure(objectName){\n"
            "       parent.StructView.window.location.replace(\"?mode=~STRUCTVIEW~&objName=\" + objectName + \"/\");\n"
            "       return true;\n"
            "   }\n"
            "   function Load(newlocation){\n"
            "       parent.location.replace(newlocation);\n"
            "       return true;\n"
            "   }\n"
            "   function LoadHttpPage(objectName){\n"
            "       parent.StructView.window.location.replace(objectName);\n"
            "       return true;\n"
            "   }\n"
            "   function OpenHttpPage(objectName){\n"
            "       window.open(objectName,'viewWindow');\n"
            "       return true;\n"
            "   }\n"
            "   function LoadCDBPage(objectName){\n"
            "       parent.StructView.window.location.replace(\"?mode=~CDBVIEW~\" + objectName);\n"
            "       return true;\n"
            "   }\n"
            "   function OpenCDBPage(objectName){\n"
            "       window.open(\"?mode=~CDBVIEW~\" + objectName,'viewWindow');\n"
            "       return true;\n"
            "   }\n"
            "</script>\n"
            "</head>\n"
            "<BODY BGCOLOR=\"#ffffff\"><H1>%s</H1><UL>\n"
            "<FORM action=\".\">\n"
            "<INPUT type=\"button\" name=\"BACK\" title=\"Go back\" value=\"BACK\" size=4 "
            "ONCLICK=\"Load('%s')\">\n"
            "<INPUT type=\"button\" name=\"REFRESH\" title=\"Refresh\" value=\"REFRESH\" size=8 "
            "ONCLICK=\"Load('.')\">\n"
            "<INPUT type=\"hidden\" name=\"ViewSelector\" value=\"%s\">\n"
            "<INPUT type=\"hidden\" name=\"SubAddress\" value=\"%s\">\n"
            "</FORM>\n"
            "<FORM method=\"GET\">\n\n"
            "<BR>\n"
            "\n"
            ,title.Buffer()
            ,SubAddress.Buffer()
            ,title.Buffer()
            ,back.Buffer()
            ,ViewSelector.Buffer()
            ,SubAddress.Buffer()
            );


        ObjectSubView(current,"",address,ViewSelector.Buffer(),0,hStream);

        hStream.Printf(
            "</FORM>\n"
            "</UL></BODY>\n");
    }

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    //copy to the client
    hStream.WriteReplyHeader(True);

    return True;
}

