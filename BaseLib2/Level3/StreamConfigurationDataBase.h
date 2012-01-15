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
 * A CDB using the multi stream interface to read and write data into
 */
#if !defined(STREAM_CONFIGURATION_DATABASE)
#define STREAM_CONFIGURATION_DATABASE

#include "System.h"
#include "FString.h"
#include "CDBExtended.h"

#define StreamConfigurationDataBaseVersion "$Id: StreamConfigurationDataBase.h,v 1.8 2007/08/22 13:40:25 fisa Exp $"

class StreamConfigurationDataBase;

OBJECT_DLL(StreamConfigurationDataBase)

class StreamConfigurationDataBase: public FString{
OBJECT_DLL_STUFF(StreamConfigurationDataBase)

protected:
    /** the CDB being modified */
    CDBExtended     cdb;

    /** current location pointer */
    FString         configName;

    /** the order of current location within tree */
    uint32          selectedStream;

public:
    /** copies the work done to the stream this far to the database */
    void Commit(){
        if (configName.Size()==0) return;

        cdb.WriteString(this->Buffer(),configName.Buffer());
        (FString &)(*this)="";
        configName="";
    }

    /** initialises with a reference cdb */
    StreamConfigurationDataBase(ConfigurationDataBase *cdb = NULL):cdb("CDB"){
        SetCDB(cdb);
    }

    /** initialises with a reference cdb */
    void SetCDB(ConfigurationDataBase *cdb = NULL){
        configName.SetSize(0);
        if (cdb != NULL)  this->cdb = *cdb;
    }

    /** destructor */
    ~StreamConfigurationDataBase(){
        Commit();
    }

    /** Get the attribute container */
    CDBExtended &GetCDB(){ return cdb; }

    /** @name Extended Attributes or Multiple Streams INTERFACE
        @{ */

    /** how many streams are available in total.
        counts from root all the leaves
    */
    virtual uint32 NumberOfStreams(){
        return cdb->Size(CDBAM_LeafsOnly);
    }

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool Switch(uint32 n){
        Commit();
        configName = "";
        selectedStream = 0xFFFFFFFF;
        CDBExtended cdb2(cdb);
        if (!cdb2->MoveToChildren(n)) return False;
        bool ret = cdb2.ReadFString(*this,"");
        cdb2->NodeName(configName);
        selectedStream = n;

        FString::Seek(0);
        return ret;
    }

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool Switch(const char *name){
        Commit();
        configName = name;
        CDBExtended cdb2(cdb);

        if (!cdb2->Move(name)){
            configName = "";
            selectedStream = 0xFFFFFFFF;
            return False;
        }
        bool ret = cdb2.ReadFString(*this,"");

//        cdb->MoveToFather();
        selectedStream = cdb2->TreePosition();

        FString::Seek(0);
        return ret;
    }

    /** which is the selected stream */
    virtual uint32 SelectedStream(){
        return selectedStream;
    }


    /** add a new stream to write to. NB selectedStream will not be up to date.... */
    bool AddStream(const char *name){
        Commit();
        if (!StreamConfigurationDataBase::Switch(name)){
            configName = name;
        }
        return True;
    }

    /** remove an existing stream . */
    virtual bool RemoveStream(const char *name){
        selectedStream = 0xFFFFFFFF;
        return cdb->Delete(name);
    }

    /** @} */
};

#endif
