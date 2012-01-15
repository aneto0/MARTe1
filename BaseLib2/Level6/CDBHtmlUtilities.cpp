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

#include "CDBHtmlUtilities.h"
#include "CDBExtended.h"
#include "HtmlStream.h"
#include "GlobalObjectDataBase.h"


static void CDBHUHtmlObjectSubSubView(ConfigurationDataBase &cdb_,const char *cdbRelativePath,const char *absolutePath,const char *selector,int level,StreamInterface &answer){

    CDBExtended &cdb = (CDBExtended &)cdb_;

    HtmlStream hmStream(answer);

    // treat the matrix case
    if (cdb->NumberOfChildren() < 0){
        int sizes[5];
        int maxDim = 4;

        cdb->GetArrayDims(sizes,maxDim,"",CDBAIM_Strict);
        int totalSize = 1;
        for (int j=0;j<maxDim;j++){
            if (sizes[j] > 1)
                totalSize *= sizes[j];
        }

        FString *strings = new FString[totalSize];
        cdb.ReadFStringArray(strings,sizes,maxDim,"");

        int indexes[5];
        maxDim--;
        int n = 0;
        for (int i = 0;i<maxDim;i++){
            indexes[i] = 0;
        }

        hmStream.SSPrintf(HtmlTagStreamMode,"TABLE");
        while(1){
            hmStream.SSPrintf(HtmlTagStreamMode,"TR");
            for (int i=0;i<sizes[maxDim];i++){
                hmStream.SSPrintf(HtmlTagStreamMode,"TD");
                hmStream.Printf(
                    "%s"
                    ,strings[n++].Buffer()
                );
                hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
            }

            hmStream.SSPrintf(HtmlTagStreamMode,"/TR");
            if (maxDim > 0){
                int ix = maxDim-1;
                while(1){
                    indexes[ix]++;
                    if (indexes[ix] == sizes[ix]) {
                        indexes[ix] = 0;
                        ix--;
                    }
                    else break;
                    if (ix < 0) break;
                }
            } else break;
            if (indexes[0] == sizes[0]) break;
        }
        hmStream.SSPrintf(HtmlTagStreamMode,"/TABLE");
        delete[] strings;
        return;
    }

    for (int i = 0;i< cdb->NumberOfChildren();i++){
        if (cdb->MoveToChildren(i)){
            FString nodeName;
            cdb->NodeName(nodeName);
            FString fullNodeName;
            if ((cdbRelativePath == NULL) || (cdbRelativePath[0] == 0)){
                fullNodeName = nodeName;
            } else {
                fullNodeName.Printf("%s.%s",cdbRelativePath,nodeName.Buffer());
            }
            FString absoluteFullNodeName;
            if (strlen(absolutePath)>0) {
                absoluteFullNodeName.Printf("%s.%s",absolutePath,fullNodeName.Buffer());
            } else {
                absoluteFullNodeName = fullNodeName;
            }

            for (int j=0;j<level;j++){
                hmStream.Printf(
                    "----"
                );
            }

            if (cdb->NumberOfChildren() < 0){
                int sizes[5];
                int maxDim = 4;

                FString nodeType;
                cdb->NodeType(nodeType);

                cdb->GetArrayDims(sizes,maxDim,"",CDBAIM_Strict);
                int totalSize = 1;
                for (int j=0;j<maxDim;j++){
                    if (sizes[j] > 1)
                        totalSize *= sizes[j];
                }

                if (totalSize == 1){
                    FString value;
                    cdb.ReadFString(value,"");

                    hmStream.Printf(
                        "%s %s = "
                        ,nodeType.Buffer()
                        ,nodeName.Buffer()
                    );
                    hmStream.SSPrintf(HtmlTagStreamMode,
                        "INPUT type=\"text\" name=\"%s\"  value=\"%s\" "
                        "ONCHANGE=ChangeValue(\"%s\",this.value);"
                        ,fullNodeName.Buffer()
                        ,value.Buffer()
                        ,fullNodeName.Buffer()
                    );
                    hmStream.SSPrintf(HtmlTagStreamMode,"BR");
                } else {
                    FString sizeString;
                    for (int j=0;j<(maxDim-1);j++){
                        sizeString.Printf("%i,",sizes[j]);
                    }
                    if (maxDim >= 1)
                        sizeString.Printf("%i",sizes[maxDim-1]);
                    else
                        sizeString.Printf("1");


                    hmStream.Printf(
                        "%s"
                        ,nodeName.Buffer()
                    );
                    hmStream.SSPrintf(HtmlTagStreamMode,
                        "A HREF=\"%s\""
                        ,absoluteFullNodeName.Buffer()
                    );
                    hmStream.Printf(
                        "ARRAY[%s]"
                        ,sizeString.Buffer()
                    );
                    hmStream.SSPrintf(HtmlTagStreamMode,"/A");
                    hmStream.SSPrintf(HtmlTagStreamMode,"BR");
                }
            } else {
                FString wrappedFullNodeName;
                wrappedFullNodeName.Printf("|%s|",fullNodeName.Buffer());

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

                    hmStream.Printf(
                        "%s"
                        ,nodeName.Buffer()
                    );
                    hmStream.SSPrintf(HtmlTagStreamMode,
                        "INPUT type=\"button\" name=\"%s\" title=\"-\" value=\"-\" size=1 "
                        "ONCLICK=\"Expand('%s')\""
                        ,nodeName.Buffer()
                        ,updatedSelector.Buffer()
                    );
                    hmStream.SSPrintf(HtmlTagStreamMode,"BR");
                    hmStream.Flush();
                    CDBHUHtmlObjectSubSubView(cdb,fullNodeName.Buffer(),absolutePath,selector,level+1,answer);
                } else {
                    FString updatedSelector;
                    if (strlen(selector)>0) {
                        updatedSelector.Printf("%s%s|",selector,fullNodeName.Buffer());
                    } else {
                        updatedSelector.Printf("%s|",fullNodeName.Buffer());
                    }
                    hmStream.SSPrintf(HtmlTagStreamMode,
                        "A HREF=\"%s\""
                        ,absoluteFullNodeName.Buffer()
                    );
                    hmStream.Printf(
                        "%s"
                        ,nodeName.Buffer()
                    );
                    hmStream.SSPrintf(HtmlTagStreamMode,"/A");
                    hmStream.SSPrintf(HtmlTagStreamMode,
                        "INPUT type=\"button\" name=\"%s\" title=\"+\" value=\"+\" size=1 "
                        "ONCLICK=\"Expand('%s')\""
                        ,nodeName.Buffer()
                        ,updatedSelector.Buffer()
                    );
                    hmStream.SSPrintf(HtmlTagStreamMode,"BR");
                }
            }
            hmStream.Flush();
            cdb->MoveToFather();
        }
    }
}

bool CDBHUHtmlObjectSubView(ConfigurationDataBase &cdb_,const char *title_,HttpStream &hStream,CDBHUMode mode){

    // what subtree to open
    FString CDBHUSelector;
    CDBHUSelector = "|";

    CDBExtended cdb(cdb_);

    const char *address = hStream.unMatchedUrl.Buffer();
    if (address != 0){
        if (strlen(address)>0){
            cdb->Move(address);
        }
    }

    if (hStream.Switch("InputCommands.CDBHUSelector")){
        hStream.Seek(0);
        hStream.GetToken(CDBHUSelector,"");
        hStream.Switch((uint32)0);
    }

    if (hStream.Switch("InputCommands.CDBHUParameter")){
        hStream.Seek(0);
        FString CDBHUParameter;
        hStream.GetToken(CDBHUParameter,"");
        hStream.Switch((uint32)0);

        if (hStream.Switch("InputCommands.CDBHUValue")){
            hStream.Seek(0);
            FString CDBHUValue;
            hStream.GetToken(CDBHUValue,"");
            hStream.Switch((uint32)0);

            if (cdb->Exists(CDBHUParameter.Buffer())){
                cdb.WriteString(CDBHUValue.Buffer(),CDBHUParameter.Buffer());
            }
        }
    }

    FString title;
    title = title_;
    if (strlen(address) > 0){
        title += '.';
        title += address;
    }

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


    if (mode & CDBHUHOSUV_Header){
        hStream.Printf(
            "<html>\n"
            "<head>\n"
            "<TITLE>%s</TITLE>\n"
            "<script type=\"text/javascript\">\n"
            "   function ChangeValue(parameter,value){\n"
            "       window.location.replace(\"./%s?CDBHUSelector=%s&CDBHUParameter=\" + parameter + \"&CDBHUValue=\" + value);\n"
            "       return true;\n"
            "   }\n"
            "   function Expand(expand){\n"
            "       window.location.replace(\"./%s?CDBHUSelector=\" + expand);\n"
            "       return true;\n"
            "   }\n"
            "</script>\n"
            "</head>\n"
            ,title.Buffer()
            ,address
            ,CDBHUSelector.Buffer()
            ,address
        );
    }
    if (mode & CDBHUHOSUV_Script){
        hStream.Printf(
            "<script type=\"text/javascript\">\n"
            "   function ChangeValue(parameter,value){\n"
            "       window.location.replace(\"./%s?CDBHUSelector=%s&CDBHUParameter=\" + parameter + \"&CDBHUValue=\" + value);\n"
            "       return true;\n"
            "   }\n"
            "   function Expand(expand){\n"
            "       window.location.replace(\"./%s?CDBHUSelector=\" + expand);\n"
            "       return true;\n"
            "   }\n"
            "</script>\n"
            ,address
            ,CDBHUSelector.Buffer()
            ,address
        );
    }
    if (mode & CDBHUHOSUV_Body){
        if ((mode & CDBHUHOSUV_NoBack) == 0){
            hStream.Printf(
                "<A HREF=\"%s\">BACK</A>\n"
                ,back.Buffer()
            );
        }
        hStream.Printf(
            "<FORM>\n"
            "<INPUT type=\"submit\" name=\"REFRESH\" value=\"REFRESH\">\n"
            "<INPUT type=\"hidden\" name=\"CDBHUSelector\" value=\"%s\">\n"
            "</FORM>\n"
            "<FORM method=\"GET\">\n\n"
            "<BR>\n"
            ,CDBHUSelector.Buffer()
        );
        CDBHUHtmlObjectSubSubView(cdb,"",address,CDBHUSelector.Buffer(),0,hStream);

        hStream.Printf(
            "</FORM>\n"
            "</UL>\n");
    }
    if (mode & CDBHUHOSUV_FullBody){
        hStream.Printf(
            "<BODY BGCOLOR=\"#ffffff\"><H1>%s</H1><UL>\n"
            ,title.Buffer()
        );
        if ((mode & CDBHUHOSUV_NoBack) == 0){
            hStream.Printf(
                "<A HREF=\"%s\">BACK</A>\n"
                ,back.Buffer()
            );
        }
        hStream.Printf(
            "<FORM>\n"
            "<INPUT type=\"submit\" name=\"REFRESH\" value=\"REFRESH\">\n"
            "<INPUT type=\"hidden\" name=\"CDBHUSelector\" value=\"%s\">\n"
            "</FORM>\n"
            "<FORM method=\"GET\">\n\n"
            "<BR>\n"
            ,CDBHUSelector.Buffer()
        );
        CDBHUHtmlObjectSubSubView(cdb,"",address,CDBHUSelector.Buffer(),0,hStream);

        hStream.Printf(
            "</FORM>\n"
            "</UL></BODY>\n");
    }


    return True;
}


