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

#include "CDBBrowserMenu.h"
#include "CDBExtended.h"
#include "HtmlStream.h"
#include "HttpUtilities.h"

OBJECTLOADREGISTER(CDBBrowserMenu,"$Id: CDBBrowserMenu.cpp,v 1.6 2008/11/10 13:32:12 fisa Exp $")



bool CDB2Setup(CDBBrowserMenu &cbm,const char *cdbName,ConfigurationDataBase &cdb){

    if (cdbName){
        FString title;
        title.Printf("%s ",cdbName);
        cbm.SetTitle(title.Buffer());
        cbm.cdbName = cdbName;
    }

    cbm.cdb = cdb;
    return True;
}

static bool CDBBrowseVector(StreamInterface &in,StreamInterface &out,ConfigurationDataBase &cdb_){

    CDBExtended &cdb = (CDBExtended &)cdb_;

    FString nodeName;
    cdb->NodeName(nodeName);

    int dim = 3;
    int size[3] = {0,0,0};
    if(!cdb->GetArrayDims(size,dim,"")){
        out.Printf("CDBBrowseVector::GetArrayDimension Failed for %s",nodeName.Buffer());
        return False;
    }

    if(dim > 1){
        out.Printf("CDBBrowseVector:: %s has dim > 1. It should be 1",nodeName.Buffer());
        return False;
    }

    FString *data = new FString[size[0]];
    if (data == NULL){
        out.Printf("CDBBrowseVector:: Failed allocating space for data (%s)",nodeName.Buffer());
        return False;
    }

    if(!cdb.ReadFStringArray(data,size,dim,"")){
        if(data)delete[] data;
        return False;
    }

    int position = 0;

    char buffer[128];
    uint32 sizeBuffer = sizeof(buffer);

    while(1){
        out.Printf(
        "###############################################################################\n"
        "##     %s                            \n"
        "###############################################################################\n"
        ,nodeName.Buffer());

        int index    = 0;
        while(((position + index ) < size[0])&&(index<16)){
            out.Printf("%s[%d]    =  %s \n",nodeName.Buffer(),index+position,data[position + index].Buffer());
            index++;
        }

        out.Printf("###############################################################################\n");
        out.Printf("0: EXIT");
        if ((size[0]-position)>16) out.Printf(" >: UP");
        if (position>0) out.Printf("<: DOWN");
        out.Printf("\n");

        sizeBuffer = sizeof(buffer);
        if (!in.Read(buffer,sizeBuffer)) SleepMsec(100);
//        if (!in.GetLine(buffer,sizeBuffer,False)) SleepMsec(100);
        else
        if (strlen(buffer)>0){
            char command = toupper(buffer[0]);
            if (command == '0') {
                if(data) delete[] data;
                return True;
            } else
            if (command == '>') {
                position += 8;
                if ((size[0] - position)<16) position = size[0] - 16;
                if (position<0) position = 0;
            } else
            if (command == '<') {
                position -= 8;
                if (position<0) position = 0;
            }
        }
    }

    if(data) delete[] data;
    return False;
}

bool CDB2Browse(StreamInterface &in,StreamInterface &out,ConfigurationDataBase &cdbRef,const char *cdbName){

    char buffer[128];
    uint32 size = sizeof(buffer);

    FString tempName;
    tempName = cdbName;

    // Points to the Node
    CDBExtended node(cdbRef);

    // Points to the Lefmost child of Node
    CDBExtended leftMostChild(node);

    int position = 0;
    while(1){
        out.Printf(
        "###############################################################################\n"
        "##     %s                            \n"
        "###############################################################################\n"
        ,tempName.Buffer());

        int nOfChildren = node->NumberOfChildren();

        if (nOfChildren > 0)
            leftMostChild->MoveToChildren(position);


        int index = 0;
        char c = 'A';

        bool hasBrother = True;
        CDBExtended browseBrothers(leftMostChild);

        while((hasBrother)&&(index<16)){

            FString tempor;
            browseBrothers->NodeName(tempor);

            int nodeChildren = browseBrothers->NumberOfChildren();

            // If it is not a leaf
            if (nodeChildren > 0){
                out.Printf("  [%c] %s \n",c,tempor.Buffer());
            } else {

                int dim      = 3;
                int size[3]  = {0,0,0};
                bool isArray = browseBrothers->GetArrayDims(size,dim,"");

                // If it is a scalar print the value
                if(dim == 0){
                    FString value;
                    browseBrothers.ReadFString(value,"");
                    out.Printf("      %s = %s\n",tempor.Buffer(),value.Buffer());
                }
                if(dim == 1){
                    out.Printf("  [%c] %s[%d]\n",c,tempor.Buffer(),size[0]);
                }
            }
            c++;
            index++;
            hasBrother = browseBrothers->MoveToBrother();
        }

        out.Printf("###############################################################################\n");
        out.Printf("0: EXIT");
        if ((nOfChildren-position)>16) out.Printf(" >: UP");
        if (position>0) out.Printf("<: DOWN");
        out.Printf("\n");

        size = sizeof(buffer);
        if (!in.Read(buffer,size)) SleepMsec(100);
        else
        if (strlen(buffer)>0){
            char command = toupper(buffer[0]);
            if (command == '0') {
                // Find if the node is the root ---> subtree path is empty
                FString path;
                node->SubTreeName(path);
                if(path == ""){
                    tempName = cdbName;
                    return True;
                }
                // It is not the root...
                node->MoveToFather();
                leftMostChild->MoveToFather(2);
                node->NodeName(tempName);
                position = 0;
            } else
            if (command == '>') {
                position += 8;
                if ((nOfChildren-position)<16) position = nOfChildren - 16;
                if (position<0) position = 0;
                // This allows the Move to children to point to the
                // Child(position)
                leftMostChild->MoveToFather();
            } else
            if (command == '<') {
                position -= 8;
                if (position<0) position = 0;
                leftMostChild->MoveToFather();
            } else {
                int index = command - 'A' + position;
                if ((index >= 0) && (index < nOfChildren)){

                    node->MoveToChildren(index);
                    int nOfChildren = node->NumberOfChildren();

                    // If it is a leaf
                    if(nOfChildren == -1){
                        CDBBrowseVector(in,out,node);
                        node->MoveToFather();
                        // This allows the Move to children to point to the
                        // Child(position)
                        leftMostChild->MoveToFather();
                    }else{
                        // Visit the node
                        leftMostChild->MoveToBrother(index - position);
                    }

                    node->NodeName(tempName);
                }
            }
        }
    }
}
//#define HttpEncode(x,y) x

