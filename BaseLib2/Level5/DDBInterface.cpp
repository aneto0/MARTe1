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

#include "DDBInterface.h"
#include "ObjectRegistryDataBase.h"
#include "FString.h"
#include "File.h"
#include "CDBExtended.h"
#include "MenuContainer.h"
#include "MenuEntry.h"

static inline bool SetBits(FString &output,uint32 from,uint32 step,uint32 to){
    for (uint32 ix = from; ix <= to; ix += step){
        FString temp;
        temp.Printf(" %d,",ix);
        output += temp;
    }
    return True;
}
static inline bool ParsePattern(uint32 &from, uint32 &step, uint32 &to ,FString &pattern){
    char token[256];

    int data[3]= {-1,-1,-1};

    // status = 0 --> no colum has been found
    // status = 1 --> one colum has been found
    // status = 2 --> two colum has been found
    // status > 2 --> error

    // Allowed sintax: x
    // Allowed sintax: x:y   --> x, x+1, x+2   ... y-1, y
    // Allowed sintax: x:y:z --> x, x+y, x+2y, ... z-y, z
    int status = 0;

    char sep;

    pattern.Seek(0);

    while (pattern.GetToken(token," :\n,;\t\r",sizeof(token),&sep,NULL)){
        switch(status){
        case 0:{
            if (strlen(token)> 0 ){
                data[0] = atoi(token);
                if(sep == ':'){
                    status++;
                }else{
                    from = data[0];
                    step = 1;
                    to   = data[0];
                    return True;
                }
            }else{
                if(sep == ':'){
                    CStaticAssertErrorCondition(FatalError,"ParsePattern Unexpected ':' at status %d",status);
                    return False;
                }
            }
        }break;
        case 1:{
            if (strlen(token)> 0 ){
                data[1] = atoi(token);
                if(sep == ':'){
                    CStaticAssertErrorCondition(FatalError,"ParsePattern more than 1 ':' !!");
                    return False;
                }else{
                    from = data[0];
                    step = 1;
                    to   = data[1];
                    return True;
                }
            }else{
                if(sep == ':'){
                    CStaticAssertErrorCondition(FatalError,"ParsePattern Unexpected ':' at status %d",status);
                    return False;
                }
            }
        }break;

            // You should never reach this case for the time being. Only the sintax a:b is supported.
            // Leave this bit of code in case we will support the a:b:c case in future.
        case 2:{
            if (strlen(token)> 0 ){
                data[2] = atoi(token);
                if(sep == ':'){
                    CStaticAssertErrorCondition(FatalError,"ParsePattern more than 2 ':' !!");
                    return False;
                }else{
                    // if step is larger than final value...maybe there is a mistake
                    if(data[1] > data[2]){
                        CStaticAssertErrorCondition(FatalError,"ParsePattern: This format may contain an error %d:%d:%d. Please check!",data[0],data[1],data[2]);
                    }
                    from = data[0];
                    step = data[1];
                    to   = data[2];
                    return True;
                }
            }else{
                if(sep == ':'){
                    CStaticAssertErrorCondition(FatalError,"ParsePattern Unexpected ':' at status %d",status);
                    return False;
                }
            }
        }break;
        default:{
            CStaticAssertErrorCondition(FatalError,"ParsePattern unknown status");
            return False;
        };
        }
    }
    return True;
}
static bool IsBasicType(const char* signalType);
static bool TypeName2TypeCode(const char* signalType, BasicTypeDescriptor& typeCode);

bool DDBIAddSignal(
    DDBInterface&         ddbi,
    const char*           signalName,
    const char*           signalType,
    DDBStoringProperties  properties){

    if ((signalName==NULL) || (strlen(signalName)==0)){
        ddbi.AssertErrorCondition(ParametersError,"DDBIAddSignal(): A signal without name cannot be added; skipping operation.");
        return False;
    }

    // Partial dimensions are not supported for the basic types only.
    if (!IsBasicType(signalType) && properties.CheckMask(DDB_Unsized)){
        ddbi.AssertErrorCondition(ParametersError,"AddSignalRecursive(): A structured signal can't use partial dimensions; skipping operation.");
        return False;
    }

    if (!ddbi.ParseRawNameAndAdd(signalName,signalType,properties)){
        ddbi.AssertErrorCondition(ParametersError,"DDBIAddSignal(): Signal %s not valid; skipping operation",signalName);
        return False;
    }

    return True;
}

const DDBSignalDescriptor* DDBISignalsList(const DDBInterface& ddbi){
    return dynamic_cast<DDBSignalDescriptor*>(ddbi.listOfSignalDescriptors.List());
}

bool DDBIObjectLoadSetup(DDBInterface&         ddbi, ConfigurationDataBase&      info, StreamInterface *err){

    CDBExtended cdb(info);

    int32 numberOfSignals = cdb->NumberOfChildren();

    for(int i = 0; i <numberOfSignals; i++ ){
        cdb->MoveToChildren(i);
        FString nodeName;
        cdb->NodeName(nodeName);

        FString signalName;
        if(!cdb.ReadFString(signalName,"SignalName")){
            ddbi.AssertErrorCondition(FatalError,"DDBInterface::ObjectLoadSetup: %s did not specify signal name.",nodeName.Buffer());
            return False;
        }

        FString signalType;
        if(!cdb.ReadFString(signalType,"SignalType")){
            ddbi.AssertErrorCondition(Warning,"DDBInterface::ObjectLoadSetup: %s did not specify signal type. Assuming Float",nodeName.Buffer());
            signalType = "float";
        }

        FString compositeName;
        if(cdb->Exists("Path")){
            FString temp;
            cdb.ReadFString(temp,"Path");
            compositeName.Printf("%s.%s",temp.Buffer(),signalName.Buffer());
        }else{
            compositeName.Printf("%s",signalName.Buffer());
        }

        DDBStoringProperties  properties = DDB_Default;
        FString flatNamed;
        cdb.ReadFString(flatNamed,"FlatNamed","False");
        if(flatNamed == "True")   properties |= DDB_FlatNamed;

        FString consecutive;
        cdb.ReadFString(consecutive,"MemoryConsecutive","False");
        if(consecutive == "True") properties |= DDB_Consecutive;

        if(!ddbi.AddSignal(compositeName.Buffer(),signalType.Buffer(),properties)){
            ddbi.AssertErrorCondition(InitialisationError,"DDBInterface::ObjectLoadSetup: failed to add signal %s of type %s to interface",compositeName.Buffer(),signalType.Buffer());
            return False;
        }

        cdb->MoveToFather();
    }

    return True;

}

bool DDBInterface::ParseRawNameAndAdd(const char* rawName,const char *signalType, DDBStoringProperties  &properties){

    int      rawNameLength = strlen(rawName);
    char     lastChar      = rawName[rawNameLength-1];

    ///////////////////////
    // Parse the rawName //
    ///////////////////////

    if(lastChar == ']'){

        //////////////////////////////////////////////////////////////
        // The string specifies the name and the size of the vector //
        //////////////////////////////////////////////////////////////

        // Only basic types are allowed in vector form
        if(!IsBasicType(signalType)){
            AssertErrorCondition(FatalError,"DDBInterface:ParseRawNameAndAdd(): Only Basic types are allowed in vector format. Rejected signal %s of type %s",rawName,signalType);
            return False;
        }

        const char *openSquareBracket = strstr(rawName,"[");
        int size = 0;
        if(openSquareBracket == NULL){
            // Wrong Format
            AssertErrorCondition(FatalError,"DDBInterface:ParseRawNameAndAdd(): Missing [ in the name format %s",rawName);
            return False;
        }

        const char *end = rawName + rawNameLength - 2;
        int   unit = 1;
        while(end != openSquareBracket){
            if(*end == ' '){
                AssertErrorCondition(FatalError,"DDBInterface:ParseRawNameAndAdd(): No space is allowed within brackets for string %s",rawName);
                return False;
            }

            if((*end < '0') || (*end > '9')){
                AssertErrorCondition(FatalError,"DDBInterface:ParseRawNameAndAdd(): Characters are not allowed within brackets for string %s",rawName);
                return False;
            }

            int val = *end - '0';
            size   += val*unit;
            unit   *= 10;
            end--;
        }
        if(size == 0){
            AssertErrorCondition(FatalError,"DDBInterface:ParseRawNameAndAdd(): The specified size [%d] is not allowed for signal %s",size,rawName);
            return False;
        }

        FString  purgedName;
        purgedName = rawName;
        purgedName.SetSize((openSquareBracket-rawName));

        return AddSignalRecursive(purgedName.Buffer(),signalType,size,-1,properties);

    }else if(lastChar == ')'){
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        // The string specifies the name and the elements of the vector that are used (starting from 0)  //
        ///////////////////////////////////////////////////////////////////////////////////////////////////

        // Only basic types are allowed in vector form
        if(!IsBasicType(signalType)){
            AssertErrorCondition(FatalError,"DDBInterface:ParseRawNameAndAdd(): Only Basic types are allowed in vector format. Rejected signal %s of type %s",rawName,signalType);
            return False;
        }

        const char *openRoundBracket = strstr(rawName,"(");
        if(openRoundBracket == NULL){
            // Wrong Format
            AssertErrorCondition(FatalError,"DDBInterface:ParseRawNameAndAdd(): Missing ( in the name format %s",rawName);
            return False;
        }

        const char *end = rawName + rawNameLength - 2;

        FString  purgedName;
        purgedName = rawName;
        purgedName.SetSize((openRoundBracket-rawName));

        // If the Interface has WriteMode Enabled all sizes must be specified
        if (ddbInterfaceDescriptor.AccessMode().CheckMask(DDB_WriteMode)){
            AssertErrorCondition(ParametersError,"ParseRawNameAndAdd(): Interface %s with write access cannot use signal %s with partial dimensions.",ddbInterfaceDescriptor.InterfaceName(),rawName);
            return False;
        }

        // if the signal is specified with round brackets the size is not declared
        properties |= DDB_Unsized;


        // Get the content of the round brackets
        FString content;
        content = openRoundBracket+1;
        content.SetSize(end-openRoundBracket);

        uint32 from = 0;
        uint32 step = 0;
        uint32 to   = 0;

        char sep;
        FString token;
        while (content.GetToken(token,",)",&sep," ")){
            if(!ParsePattern(from,step,to,token)){
                AssertErrorCondition(FatalError,"DDBInterface:NameHasValidExtension(): Failed Parsing %s, %s",rawName,token.Buffer());
                return False;
            }
            // Add consecutive elements to the signal list
            if(!AddSignalRecursive(purgedName.Buffer(),signalType,to-from+1,from,properties)){
                AssertErrorCondition(FatalError,"DDBInterface:NameHasValidExtension(): AddSignalRecursive Failed while adding signal %s",rawName);
                return False;
            }
            token = "";
        }

    }else{
        //////////////////////////////////////////////////////////////
        // The string specifies only the name and assumes size of 1 //
        //////////////////////////////////////////////////////////////

        FString  purgedName;
        purgedName = rawName;
        if(!AddSignalRecursive(purgedName.Buffer(),signalType,1,-1,properties)){
            AssertErrorCondition(FatalError,"DDBInterface:NameHasValidExtension(): AddSignalRecursive Failed while adding signal %s",rawName);
            return False;
        }
    }

    return True;
}

bool DDBInterface::AddSignalRecursive(const char*           signalName,
                                      const char*           signalType,
                                      int                   signalSize,
                                      int                   from,
                                      DDBStoringProperties& properties){

    // Basic case.
    if (IsBasicType(signalType)){
        // Check that the signal type at this point is supported.
        BasicTypeDescriptor typeCode;
        if (!TypeName2TypeCode(signalType, typeCode)) return False;
        return AddUnstructuredSignal(signalName,signalSize,from,typeCode,properties);
    }
    else{
        if((signalSize > 1)){
            AssertErrorCondition(FatalError,"DDBInterface::AddSignalRecursive(): Arrays of not basic type are not supported. Type %s is not a basic type",signalType);
            return False;
        }

        ObjectRegistryItem *ori = ObjectRegistryDataBaseFind(signalType);
        if (ori == NULL){
            AssertErrorCondition(FatalError,"DDBInterface::AddSignalRecursive(): Cannot find type %s; skipping operation.",signalType);
            return False;
        }

        ClassStructure *structure = ori->structure;
        if (structure == NULL){
            AssertErrorCondition(FatalError,"DDBInterface::AddSignalRecursive: Type %s has no structurer definition; skipping operation.",signalType);
            return False;
        }

        ClassStructureEntry **cse = structure->Members();
        if (cse != NULL)
            while (cse[0] != NULL){
                if (strlen(cse[0]->modif) != 0){
                    AssertErrorCondition(FatalError,"DDBInterface::AddSignalRecursive:: class structure entry %s.%s is a pointer or reference: %s ",signalType,cse[0]->name,cse[0]->modif);
                    return False;
                }

                FString compositeName;
                if (properties.CheckMask(DDB_FlatNamed)){
                    compositeName.Printf("%s",cse[0]->name);
                }else{
                    compositeName.Printf("%s.%s",signalName,cse[0]->name);
                }
                // calculate size of vector start with number of elements specified
                // rimpiazzare con la nuova routine di filippo.
                int dimensions = 1;
                for (int i=0 ; i < cse[0]->NumberOfDimensions(); i++){
                    if (cse[0]->sizes[i] > 1) dimensions *= cse[0]->sizes[i];
                }

                if (!AddSignalRecursive(compositeName.Buffer(),cse[0]->type,dimensions,-1,properties)){
                    AssertErrorCondition(FatalError,"DDBInterface::AddSignalRecursive: Failed adding signal %s of type %s",compositeName.Buffer(),cse[0]->type);
                    return False;
                }

                cse++;
            }

        return True;
    }
}

bool DDBInterface::AddUnstructuredSignal(const char*                   signalName,
                                         const int                     signalSize,
                                         const int                     from,
                                         const BasicTypeDescriptor     typeCode,
                                         DDBStoringProperties&         properties){

    if ((signalName==NULL) || (strlen(signalName)==0)){
        AssertErrorCondition(ParametersError,"DDBInterface::AddUnstructuredSignal(): A signal without name cannot be added; skipping operation.");
        return False;
    }

    uint32 index = 0;
    DDBSignalDescriptor* signal = Find(signalName, index);
    if ((signal != NULL)&&(ddbInterfaceDescriptor.AccessMode().CheckMask(DDB_WriteMode|DDB_ExclusiveWriteMode))){
        AssertErrorCondition(IllegalOperation,"DDBInterface::AddUnstructuredSignal(): Signal %s cannot be added more than one time; skipping operation.",signalName);
        return False;
    }

    // Creates a new signalDescriptor and add it to the listOfSignalDescriptors
    DDBSignalDescriptor* newSignal = new DDBSignalDescriptor(signalName,signalSize,from,typeCode,properties);
    if(newSignal == NULL){
        AssertErrorCondition(IllegalOperation,"DDBInterface::AddUnstructuredSignal(): Failed allocating Signal Descriptor for signal %s",signalName);
        return False;
    }

    listOfSignalDescriptors.ListAdd(newSignal);

    return True;
}

bool DDBIFinalise(DDBInterface&         ddbi){

    if (!ddbi.finalised){
        int numberOfSignals    = ddbi.listOfSignalDescriptors.ListSize();

        ddbi.ddbSignalPointers      = new DataBufferPointer[numberOfSignals];

        if((ddbi.ddbSignalPointers == NULL)){
            ddbi.AssertErrorCondition(InitialisationError,"DDBInterface::Finalise: Failed allocating memory for %d elements",numberOfSignals+1);
            return False;
        }

        // Compute total size and allocate storing buffer
        int totalBufferSize = 0;

        const LinkedListable *item = ddbi.listOfSignalDescriptors.List();
        while(item != NULL){
            const DDBSignalDescriptor *signal = dynamic_cast<const DDBSignalDescriptor *>(item);
            if(signal == NULL){
                ddbi.AssertErrorCondition(InitialisationError,"DDBInterface::Finalise: dynamic_cast to DDBSignalDescriptor failed");
                delete[] ddbi.ddbSignalPointers;
                ddbi.ddbSignalPointers = NULL;
                return False;
            }

            totalBufferSize +=  signal->SignalSize()*signal->SignalTypeCode().ByteSize();
            item = item->Next();
        }

        ddbi.bufferWordSize = totalBufferSize/sizeof(int32);
        ddbi.buffer = (int32 *)malloc(totalBufferSize);
        if(ddbi.buffer == NULL){
            ddbi.AssertErrorCondition(InitialisationError,"DDBInterface::Finalise: Failed allocating %i bytes of memory for interface %s's buffer",totalBufferSize*sizeof(int),ddbi.ddbInterfaceDescriptor.InterfaceName());
            delete[] ddbi.ddbSignalPointers;
            ddbi.ddbSignalPointers = NULL;
            return False;
        }

        memset(ddbi.buffer                ,0,totalBufferSize);
        ddbi.finalised=True;

    }
    return True;
}

bool DDBInterface::Link(const char* signalName, int32* ddbSignalPointer){

    if (finalised){

        ///////////////////////
        // Search for signal //
        ///////////////////////

        bool found = False;
        uint32                      matchingDescriptorIndex    = 0;
        DDBSignalDescriptor*        matchingDescriptor         = NULL;
        int counter = 1;
        while(!found){
            matchingDescriptor         = Find(signalName,matchingDescriptorIndex);
            if(matchingDescriptor == NULL){
                AssertErrorCondition(FatalError,"DDBInterface::Link(): Find failed to find %dth element %s in the list",counter,signalName);
                return False;
            }
            // if the signal match has been found and the relative pointer has not been initialized return true..
            if((matchingDescriptor != NULL) && (ddbSignalPointers[matchingDescriptorIndex].GetPointer() == NULL)){
                found = True;
            }else{
                matchingDescriptorIndex++;
            }
        }

        //////////////////////////////
        // Get Basic Type dimension //
        //////////////////////////////

        int32 sizeIn32BitWords                          = matchingDescriptor->SignalTypeCode().Word32Size();
        ddbSignalPointers[matchingDescriptorIndex].SetSignalParameters(ddbSignalPointer + sizeIn32BitWords * matchingDescriptor->SignalFromIndex(),sizeIn32BitWords * matchingDescriptor->SignalSize());

        return True;
    }

    AssertErrorCondition(FatalError,"DDBInterface::Link(): Operation not possible until the interface has been finalised.");
    return False;
}

void DDBInterface::Print(StreamInterface& s) const{
    ddbInterfaceDescriptor.Print(s);
    s.Printf("\nListOfSignalDescriptors for interface %s:\n",ddbInterfaceDescriptor.InterfaceName());

    const DDBSignalDescriptor* ddbsignal = SignalsList();
    int   index = 0;
    while(ddbsignal != NULL){
        if(ddbSignalPointers != NULL){
            ddbsignal->Print(s);
            s.Printf("At 0x%x to 0x%x\n", ddbSignalPointers[index].GetPointer(),ddbSignalPointers[index].GetPointer() + ddbSignalPointers[index].GetSize());
            index++;
        }else{
            ddbsignal->Print(s);
            s.Printf("\n");
        }

        ddbsignal = ddbsignal->Next();
    }
}

bool DDBIObjectSaveSetup(DDBInterface&         ddbi, ConfigurationDataBase&      info, StreamInterface *err){

    CDBExtended cdb(info);
    cdb->AddChildAndMove(ddbi.InterfaceName());
    cdb.WriteString(ddbi.ClassName(),"Class");

    const DDBSignalDescriptor* ddbsignal = ddbi.SignalsList();
    int   index = 0;
    while(ddbsignal != NULL){
        if(ddbi.ddbSignalPointers != NULL){
            FString x;
            x.Printf("Signal%d",index);
            cdb->AddChildAndMove(x.Buffer());
            ddbsignal->ObjectSaveSetup(cdb,err);
            if(ddbi.ddbSignalPointers[index].GetPointer() != 0){
                cdb.WritePointer(ddbi.ddbSignalPointers[index].GetPointer(),"PositionInMemory");
            }else{
                // Read the Entry from the previous element
                int32 previousPosition = 0;

                cdb->MoveToBrother(-1);
                cdb.ReadInt32(previousPosition,"PositionInMemory");
                cdb->MoveToBrother(1);
                cdb.WriteInt32((previousPosition + (ddbsignal->SignalTypeCode().Word32Size())*ddbsignal->SignalSize()),"PositionInMemory");
            }
            cdb->MoveToFather();
            index++;
        }else{
            ddbsignal->ObjectSaveSetup(cdb,err);
        }

        ddbsignal = dynamic_cast<const DDBSignalDescriptor*>(ddbsignal->Next());
    }

    cdb->MoveToFather();
    return True;

}

bool DDBInterface::ResetPointerList(){
    if(finalised){
        for(uint32 i = 0; i < listOfSignalDescriptors.ListSize(); i++){
            ddbSignalPointers[i].SetSignalParameters(NULL,ddbSignalPointers[i].GetSize());
        }
        return True;
    }
    AssertErrorCondition(InitialisationError,"ResetPointerList: Cannot reinitialize an interface %s which is not finalized",ddbInterfaceDescriptor.InterfaceName());
    return False;
};

bool DDBInterface::CheckConsecutyAndOptimizeInterface(){
    // Check consecutivity
    if(finalised){
        int numberOfSignals  = listOfSignalDescriptors.ListSize();
        // Get the List of Descriptors
        LinkedListable *list = listOfSignalDescriptors.List();
        // Pointer to the previous descriptor
        DDBSignalDescriptor *descCons = dynamic_cast<DDBSignalDescriptor *>(list);
        if(descCons == NULL){
            AssertErrorCondition(InitialisationError,"CheckConsecutyAndOptimizeInterface: dynamic_cast DDBSignalDescriptor * failed for signal in interface %s",ddbInterfaceDescriptor.InterfaceName());
            return False;
        }

        // Skip the first entry since there is no point in checking whether it is consecutive
        list = list->Next();

        for(int i = 1; i < numberOfSignals; i++){
            int32 *pointer      = ddbSignalPointers[i-1].GetPointer();
            int32 size          = ddbSignalPointers[i-1].GetSize();
            int32 *nextPointer  = ddbSignalPointers[i].GetPointer();
            DDBSignalDescriptor *desc = dynamic_cast<DDBSignalDescriptor *>(list);
            if(desc == NULL){
                AssertErrorCondition(InitialisationError,"CheckConsecutyAndOptimizeInterface: dynamic_cast DDBSignalDescriptor * failed for signal in interface %s",ddbInterfaceDescriptor.InterfaceName());
                return False;
            }
            // If signals are consecutive
            if(((pointer + size) !=  nextPointer)&&(desc->SignalStoringProperties().CheckMask(DDB_Consecutive))){
                FString compoundNameDescCons;
                FString compoundNameDesc;
                descCons->GetCompoundName(compoundNameDescCons);
                desc->GetCompoundName(compoundNameDesc);
                AssertErrorCondition(InitialisationError,"CheckConsecutyAndOptimizeInterface: Signal %s is not consecutive to signal %s in interface %s",compoundNameDesc.Buffer(),compoundNameDescCons.Buffer(),ddbInterfaceDescriptor.InterfaceName());
                return False;
            }
            // Update Pointer to previous Descriptor
            descCons = desc;
            list = list->Next();
        }
    }

    // If everything is ok Optimize the Interface
    if(finalised){
        // Reduce the number of accesses to memory
        int numberOfSignals = listOfSignalDescriptors.ListSize();
        for(int i = numberOfSignals - 2; i >= 0 ; i--){
            int32 *pointer      = ddbSignalPointers[i].GetPointer();
            int32 size          = ddbSignalPointers[i].GetSize();
            int32 *nextPointer  = ddbSignalPointers[i+1].GetPointer();
            // If signals are consecutive
            if((pointer + size) ==  nextPointer){
                int deltaSize = ddbSignalPointers[i+1].GetSize();
                ddbSignalPointers[i].SetSignalParameters(pointer,size+deltaSize);
                ddbSignalPointers[i+1].SetSignalParameters(NULL,0);
            }
        }

        /////////////////////
        // Compact Entries //
        /////////////////////
        int nextNullEntry = 0;
        int entry         = 0;
        int arraySize     = listOfSignalDescriptors.ListSize();
        while(1){

            // Find Consecutive Null Entry
            while(nextNullEntry < arraySize){
                if(ddbSignalPointers[nextNullEntry].GetPointer()==NULL)break;
                nextNullEntry++;
            }
            if(nextNullEntry == arraySize)return True;

            // Find the next non empty entry
            entry = nextNullEntry + 1;
            while(entry < arraySize){
                if(ddbSignalPointers[entry].GetPointer()!=NULL)break;
                entry++;
            }
            if(entry == arraySize)       return True;


            int32 *pointer  = ddbSignalPointers[entry].GetPointer();
            int32 size      = ddbSignalPointers[entry].GetSize();
            ddbSignalPointers[nextNullEntry].SetSignalParameters(pointer,size);
            ddbSignalPointers[entry].SetSignalParameters(NULL,0);
            // Start from
            nextNullEntry++;
        }


        return True;
    }
    AssertErrorCondition(InitialisationError,"CheckConsecutyAndOptimizeInterface: Cannot optimize interface %s which is not finalized",ddbInterfaceDescriptor.InterfaceName());
    return False;
}

DDBSignalDescriptor* DDBInterface::Find(const char* signalName, uint32 &index){
    if(index > listOfSignalDescriptors.ListSize()){
        AssertErrorCondition(FatalError,"Try to access element %s at %d out of the list[%d].",signalName,index,listOfSignalDescriptors.ListSize());
        return NULL;
    }

    // Skip the first 'index' elements
    int32 i = index;
    LinkedListable *element = listOfSignalDescriptors.List();
    while(i>0){
        element = element->Next();
        i--;
    }

    while(element != NULL){
        DDBSignalDescriptor *descriptor = dynamic_cast<DDBSignalDescriptor*>(element);
        if(descriptor == NULL){
            AssertErrorCondition(FatalError,"Find: dynamic cast<DDBSignalDescriptor*> failed on list of signal descriptor");
            return NULL;
        }
        if(strcmp(descriptor->SignalName(),signalName)==0){
            return descriptor;
        }
        index++;
        element = element->Next();
    }

    return NULL;
}

const DDBSignalDescriptor* DDBIFind(const DDBInterface&         ddbi, const char* signalName){

    if(signalName == NULL) return NULL;

    LinkedListable *element = ddbi.listOfSignalDescriptors.List();
    while(element != NULL){
        DDBSignalDescriptor *descriptor = dynamic_cast<DDBSignalDescriptor*>(element);
        if(descriptor == NULL){
            ddbi.AssertErrorCondition(FatalError,"Find: dynamic cast<DDBSignalDescriptor*> failed on list of signal descriptor");
            return NULL;
        }
        if(strcmp(descriptor->SignalName(),signalName)==0){
            return descriptor;
        }
	// Direct match not found : check for array type partial match.
        int leng = strlen(descriptor->SignalName());
        int index = strncmp(signalName,descriptor->SignalName(), leng);
        if(index == 0){ // Part of the signal is equal. Check if ( or [
            if((signalName[leng] == '(') || (signalName[leng] == '[')){
                return descriptor;
            }
        }
        element = element->Next();
    }

    return NULL;
}


bool  DDBISignalRename(DDBInterface& ddbi,
                       const char* signalName,
                       const char* newSignalName){

    if(ddbi.finalised){
        CStaticAssertErrorCondition(FatalError,"SignalRename: Cannot rename signal %s in finalised interface %s",signalName,ddbi.ddbInterfaceDescriptor.InterfaceName());
        return False;
    }

    LinkedListable *item = ddbi.listOfSignalDescriptors.List();
    bool          found = False;
    while(item != NULL){
        DDBSignalDescriptor* descriptor = dynamic_cast<DDBSignalDescriptor*>(item);
        if(descriptor == NULL){
            CStaticAssertErrorCondition(FatalError,"SignalRename: dynamic cast to DDBSignalDescriptor* failed for interface %s",ddbi.ddbInterfaceDescriptor.InterfaceName());
            return False;
        }
        // Check if it is a complete match
        if(strcmp(descriptor->SignalName(),signalName)==0){
            descriptor->SignalRename(newSignalName);
            found = True;
        }else{
            // Check composite name
            FString temp;
            temp.Printf(".%s",signalName);
            const char *needle = strstr(descriptor->SignalName(), temp.Buffer());
            if(needle != NULL){
                const char *needle2 = needle + temp.Size();
                if(*needle2 == 0){
                    CStaticAssertErrorCondition(Information,"SignalRename: Renaming signal %s in interface %s as %s",descriptor->SignalName(), ddbi.ddbInterfaceDescriptor.InterfaceName(), newSignalName);
                    descriptor->SignalRename(newSignalName);
                    found = True;
                }
            }
        }
        item = item->Next();
    }

    if(!found){
        CStaticAssertErrorCondition(FatalError,"DDBISignalRename: Signal %s cannot be found in iterface %s",signalName,ddbi.ddbInterfaceDescriptor.InterfaceName());
        return False;
    }

    return True;
}

/** Calculates the typeCode of a registered elementary type. */
static bool TypeName2TypeCode(const char* signalType, BasicTypeDescriptor& typeCode){

    ObjectRegistryItem *ori = ObjectRegistryDataBaseFind(signalType);
    if (ori == NULL){
        CStaticAssertErrorCondition(FatalError,"DDBInterface::TypeName2TypeCode(): Type %s is not registered.",signalType);
        return False;
    }

    ClassStructure *structure = ori->structure;
    if (structure == NULL){
        CStaticAssertErrorCondition(FatalError,"DDBInterface::TypeName2TypeCode(): Type %s has no structured definition.",signalType);
        return False;
    }

    if (structure->Members() != NULL){
        CStaticAssertErrorCondition(FatalError,"DDBInterface::TypeName2TypeCode(): Only elementary types have a typeCode. %s is not an elementary type.",signalType);
        return False;
    }

    if(!structure->IsBasicType(typeCode)){
        CStaticAssertErrorCondition(FatalError,"DDBInterface::TypeName2TypeCode(): Elementary type %s is not supported.", signalType);
        return False;
    }

    return True;
}

static bool IsBasicType(const char* signalType){

    ObjectRegistryItem *ori = ObjectRegistryDataBaseFind(signalType);
    if (ori == NULL){
        CStaticAssertErrorCondition(FatalError,"DDBIAddSignalRecursive(): Cannot find type %s; skipping operation.",signalType);
        return False;
    }

    ClassStructure *structure = ori->structure;
    if (structure == NULL){
        CStaticAssertErrorCondition(FatalError,"DDBIAddSignalRecursive(): Type %s has no structurer definition; skipping operation.",signalType);
        return False;
    }

    if (structure->Members() == NULL){
        return True;
    }
    else{
        return False;
    }
}

int32 DDBIBufferWordSize(const DDBInterface& ddbi){
    if(ddbi.IsFinalised())return ddbi.bufferWordSize;
    int totalBufferSize = 0;

    const LinkedListable *item = ddbi.listOfSignalDescriptors.List();
    while(item != NULL){
        const DDBSignalDescriptor *signal = dynamic_cast<const DDBSignalDescriptor *>(item);
        if(signal == NULL){
            ddbi.AssertErrorCondition(InitialisationError,"DDBInterface::Finalise: dynamic_cast to DDBSignalDescriptor failed");
            return False;
        }

	totalBufferSize +=  signal->SignalSize()*(signal->SignalTypeCode().ByteSize());
        item = item->Next();
    }

    return totalBufferSize/sizeof(int32);
}

bool  DDBIBrowseMenu(StreamInterface &in,StreamInterface &out,void *userData){
    DDBInterface *ddbi = (DDBInterface *)userData;
    if(ddbi == NULL) return False;
    GCRTemplate<MenuContainer> menu(GCFT_Create);
    menu->SetTitle(ddbi->InterfaceName());

    const LinkedListable *item = ddbi->listOfSignalDescriptors.List();
    while(item != NULL){
        const DDBSignalDescriptor *signal = dynamic_cast<const DDBSignalDescriptor *>(item);
        if(signal == NULL){
            ddbi->AssertErrorCondition(InitialisationError,"DDBInterface::DDBIBrowseMenu: dynamic_cast to DDBSignalDescriptor failed");
            return False;
        }

        GCRTemplate<MenuEntry> entry(GCFT_Create);
        entry->SetTitle(signal->SignalName());
        entry->SetUp(DDBSDBrowse, NULL, NULL, (void *)signal);
        menu->Insert(entry);

        item = item->Next();
    }

    return menu->TextMenu(in, out);
}

bool  DDBIDumpMenu  (StreamInterface &in,StreamInterface &out,void *userData){
    DDBInterface *ddbi = (DDBInterface *)userData;
    if(ddbi == NULL) return False;

    ConfigurationDataBase cdb;
    if(!ddbi->ObjectSaveSetup(cdb, &out)){
        out.Printf("DDBInterface::ObjectSaveSetup: Failed for DDBInterface %s", ddbi->InterfaceName());
        return False;
    }
    FString fileName = ddbi->InterfaceName();
    fileName += ".cfg";
    File  ddbiDump;
    if(!ddbiDump.OpenNew(fileName.Buffer())){
        out.Printf("DDBInterface::ObjectSaveSetup: Failed Opening File %s", fileName.Buffer());
        return False;
    }

    cdb->WriteToStream(ddbiDump);

    ddbiDump.Close();

    return True;
}



OBJECTREGISTER(DDBInterface,"$Id: DDBInterface.cpp,v 1.33 2010/11/03 12:55:37 astephen Exp $")

