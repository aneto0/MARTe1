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

#include "Object.h"
#include "FString.h"
#include "ObjectRegistryDataBase.h"
#include "ObjectRegistryTools.h"

// standard
const int KC1DB_StatusIdent = 0x1111;
// after [
const int KC1DB_Index       = 0x2222;
// after []
const int KC1DB_StatusEnd   = 0x3333;
// potential pointer
const int KC1DB_StatusPtr   = 0x4444;


bool ObjectRegistryEvaluateAddress(const char *type,char *&address,const char *expression, ClassStructureEntry &dataType,StreamInterface *error){

    // the current structure being evaluated
    ClassStructure      *cs             = NULL;
    // the remaining part of the expression under evaluation
    FString             toBeEvaluated;
    //
    int                 status          = KC1DB_StatusIdent;


    // no way to describe this member
    cs = ObjectRegistryDataBaseFindStructure(type);
    if (cs==NULL) {
        if (error) error->Printf("%s cannot be found or is not a structure",type);
        else CStaticAssertErrorCondition(ParametersError,"ObjectRegistryEvaluateAddress:%s cannot be found or is not a structure",type);
        return False;
    }
    dataType.SetUp(type,"*",1,0,0,0,0,"base",cs->Size(),0);

    toBeEvaluated = expression;
    toBeEvaluated.Seek(0);

    FString token;
    char term;
    while (toBeEvaluated.GetToken(token,".[]",&term)){
        switch (status){
            // expect a field name
            case KC1DB_StatusIdent:{
                if (token.Size()==0){
                    if (error) error->Printf("expecting a file name found nothing");
                    else CStaticAssertErrorCondition(ParametersError,"ObjectRegistryEvaluateAddress:expecting a file name found nothing");
                    return False;
                }

                int ix = 0;

                ClassStructureEntry **members = cs->Members();

                if (members != NULL)
                    while((members[ix]!=NULL)&&(strcmp(members[ix]->name,token.Buffer())!=0))ix++;
                if ((members == NULL) || (members[ix]==NULL)) {
                    if (error) error->Printf("member %s not found within %s",token.Buffer(),dataType.type);
                    else CStaticAssertErrorCondition(ParametersError,"ObjectRegistryEvaluateAddress: member %s not found within %s",token.Buffer(),dataType.type);
                    return False;
                }
                // get info about member
                dataType = *(members[ix]);
                // steo to the start of the member and
                address += dataType.pos;

                switch(term){
                    // step to the next structure
                    case '.':{
                        cs = ObjectRegistryDataBaseFindStructure(dataType.type);
                        if (cs==NULL) {
                            if (error) error->Printf("%s cannot be found or is not a structure",dataType.type);
                            else CStaticAssertErrorCondition(ParametersError,"ObjectRegistryEvaluateAddress:%s cannot be found or is not a structure",dataType.type);
                            return False;
                        }
                    } break;
                    case '[':{
                        status = KC1DB_Index;
                    } break;
                    case ']':{
                        if (error) error->Printf("expecting a . or a [ but found a ]");
                        else CStaticAssertErrorCondition(ParametersError,"ObjectRegistryEvaluateAddress:expecting a . or a [ but found a ]");
                        return False;
                    } break;
                    case 0:{
                        return True;
                    } break;
                }
            } break;
            // evaluate a vector addressing
            case KC1DB_Index:{

                // convert string into number
                int index = atoi(token.Buffer());

                // check range
                if ((index < 0) || (index >= dataType.sizes[0])){
                    if (error) error->Printf("requested item[%i] on array[%i]",index,dataType.sizes[0]);
                    else CStaticAssertErrorCondition(ParametersError,"ObjectRegistryEvaluateAddress:requested item[%i] on array[%i]",index,dataType.sizes[0]);
                    return False;
                }
                // evaluate array row size
                int ix=0;
                while ((dataType.sizes[ix]>0)&&(ix<CSE_MAXSIZE))index*=dataType.sizes[ix++];

                // get structure in order to get size
                ClassStructure *cs2 = ObjectRegistryDataBaseFindStructure(dataType.type);
                if (cs2==NULL) {
                    if (error) error->Printf("%s cannot be found or is not a structure",dataType.type);
                    else CStaticAssertErrorCondition(ParametersError,"ObjectRegistryEvaluateAddress:%s cannot be found or is not a structure",dataType.type);
                    return False;
                }

                // move to selected array position
                index *= cs2->Size();
                address += index;

                // remove one dimension by moving indexes
                int i;
                for (i=0;i<CSE_MAXSIZE-1;i++){
                    dataType.sizes[ix] = dataType.sizes[ix+1];
                }
                if (dataType.sizes[0]<=0) dataType.sizes[0] = 1;

                switch(term){
                    case 0:
                    case '.':
                    case '[':{
                        if (error) error->Printf("expecting a ] but found a %c",term);
                        else CStaticAssertErrorCondition(ParametersError,"ObjectRegistryEvaluateAddress:expecting a ] but found a %c",term);
                        return False;
                    } break;
                    case ']':{
                        status = KC1DB_StatusEnd;
                    } break;
                }

            } break;
            case KC1DB_StatusEnd:{
                if (token.Size()>0){
                    if (error) error->Printf("expecting a [ or a . but found %s",token.Buffer());
                    else CStaticAssertErrorCondition(ParametersError,"ObjectRegistryEvaluateAddress:expecting a [ or a . but found %s",token.Buffer());
                    return False;
                }
                switch(term){
                    case '.':{
                        cs = ObjectRegistryDataBaseFindStructure(dataType.type);
                        if (cs==NULL) {
                            if (error) error->Printf("%s cannot be found or is not a structure",dataType.type);
                            else CStaticAssertErrorCondition(ParametersError,"ObjectRegistryEvaluateAddress:%s cannot be found or is not a structure",dataType.type);
                            return False;
                        }
                        status = KC1DB_StatusIdent;
                    } break;
                    case '[':{
                        status = KC1DB_Index;
                    } break;
                    case ']':{
                        if (error) error->Printf("expecting a [ or a . but found a %c",term);
                        else CStaticAssertErrorCondition(ParametersError,"ObjectRegistryEvaluateAddress:expecting a [ or a . but found a %c",term);
                        return False;
                    } break;
                    case 0:{
                        return True;
                    } break;
                }

            } break;
            default:{
                if (error) error->Printf("Internal error Status = %x ???",status);
                else CStaticAssertErrorCondition(ParametersError,"Internal error Status = %x ???",status);
                return False;
            }
        }

        token.SetSize(0);
    }
    return True;
}

