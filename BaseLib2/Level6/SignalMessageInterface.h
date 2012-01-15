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
 * @brief provides all the required functionality to retrieve signals from other objects using a message protocol
 */
#if !defined(SIGNAL_MESSAGE_INTERFACE_H)
#define SIGNAL_MESSAGE_INTERFACE_H

#include "ConfigurationDataBase.h"
#include "CDBExtended.h"
#include "MessageHandler.h"
#include "SignalInterface.h"
#include "CDBBrowserMenu.h"
#include "GCReferenceContainer.h"
#include "GlobalObjectDataBase.h"


class SignalMessageInterface;
/**
 * C function definitions
 */
extern "C" {
    /** 
      * retrieves from cache or from the collectors
      * @param smi the SignalMessageInterface object
      * @param signalName the signal name as returned by the listing AUTODETECT method
      * @return the signal data
      */
    GCRTemplate<SignalInterface> SMIGetSignal(SignalMessageInterface &smi, const char *signalName);
    
    /**
     * @sa SignalMessageInterface::ProcessMessage
     */
    bool SMIProcessMessage(SignalMessageInterface &smi, GCRTemplate<MessageEnvelope> envelope);

};

class SignalMessageInterface : public MessageHandler, public GCReferenceContainer{

    friend GCRTemplate<SignalInterface> SMIGetSignal(SignalMessageInterface &smi, const char *signalName);
    friend bool SMIProcessMessage(SignalMessageInterface &smi, GCRTemplate<MessageEnvelope> envelope);

private:
    /** timeout on wait for a signal */
    TimeoutType                     timeout;

    /** keeps the last read signals */
    GCRTemplate<GCReferenceContainer> cache;

    /** max n of signals in cache */
    int                             maxCacheSize;

    /** The root where to look and ask for signals. For some applications the GlobalObjectDataBase might be too large*/
    FString                         signalSearchRoot;

    /** Protects the get signal, otherwise there are concurrent accesses to the signal cdb*/
    MutexSem                        signalMux;

protected:
    /** The database with the signals*/
    ConfigurationDataBase           signalDataBase;


    /**
     * @sa SMIGetSignal
     */
    GCRTemplate<SignalInterface> GetSignal(const char *signalName){
        return SMIGetSignal(*this, signalName);
    }

    /**
     * Removes illegal characters from signal names
     * @param toBuild an FString without the illegal characters
     * @param signalName the original signal name
     * @param illegalCharacters the characters to be removed
     */
    void RemoveIllegalCharacters(FString &toBuild, FString &signalName, const char *illegalCharacters) {
        toBuild.SetSize(0);
        signalName.Seek(0);
        while(signalName.GetToken(toBuild, illegalCharacters));
    }
public:
    SignalMessageInterface(){
        maxCacheSize = 4;
        timeout.msecTimeout = 1000;
        GCRTemplate<CDBBrowserMenu> cdbBrowserMenu(GCFT_Create);
        if (cdbBrowserMenu.IsValid()){
            cdbBrowserMenu->LinkTo("SignalDataBase",signalDataBase);
            cdbBrowserMenu->SetObjectName("SignalDataBase");
            Insert(cdbBrowserMenu);
        } else {
            AssertErrorCondition(Warning,"could not create a cdbBrowserMenu");
        }

        GCRTemplate<GCReferenceContainer> myCache(GCFT_Create);
        if (myCache.IsValid()){
            cache = myCache;
            cache->SetObjectName("Cache");
            Insert(cache);
        } else {
            AssertErrorCondition(Warning,"could not create a cache");
        }

        signalMux.Create();
    }

    virtual     bool                ObjectLoadSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){

        bool ret = True;
        ret = ret && GCReferenceContainer::ObjectLoadSetup(info,err);

        CDBExtended cdbx(info);

        //Check if a path for a different signal search root was specified
        if(cdbx.ReadFString(signalSearchRoot, "SignalSearchRoot"));

        if (!cdbx.ReadInt32((int32 &)timeout.msecTimeout,"Timeout",1000)){
            AssertErrorCondition(Warning,"Timeout is set to default 1second");
        }
        
        return ret;
    }

    /**
        This interface operates on Messages with code 0
        The meaningful contents are
            AUTODETECT
                it first wipes the list of signal
                searches for GAMS with MessageHandlerInterface that reply
                to the question LISTSIGNALS with a user message ADDSIGNAL containing
                a number of GCNString objects (one for each signal)
                The object name should match the signal name....
                The string should contain the name of the signal the type and size
                // This is a processed version of the signalName
                // Any non CDB valid character are replaced by #XX wher XX is the hex
                Name = SIGNALNAME\n
                DataType=TYPE\n
                MaxSize=SIZE\n
                Dimensions= { d1 d2 ... } // opzionale
                (This is a CDB compatible format)
                TYPE is the string rapresentation of the BasicType
                SIZE is the max number of object that will be returned
                This information is then converted in CDB format and stored
                in the signalDataBase
     */
    virtual bool ProcessMessage(GCRTemplate<MessageEnvelope> envelope){
        return SMIProcessMessage(*this, envelope);
    }

};
#endif

