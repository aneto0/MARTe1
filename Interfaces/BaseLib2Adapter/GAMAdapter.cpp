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
 * $Id: RelayLogger.cpp 3 2012-01-15 16:26:07Z aneto $
 *
**/

#include "ConfigurationDataBase.h"
#include "DDB.h"
#include "DDBInputInterface.h"
#include "DDBOutputInterface.h"
#include "GAM.h"
#include "GAMAdapter.h"
#include "GlobalObjectDataBase.h"
#include "MenuContainer.h"
#include "LoggerService.h"

namespace BaseLib2 {
/**
 * @brief TODO
 */
struct GAMAdapterInfo {
    FString gamName;
    DDB *ddb;
    GAM *gam;
    DDBInputInterface *outputFromGAM;
    DDBOutputInterface *inputToGAM;
};
static GAMAdapterInfo *gamList = NULL;
static uint32 gamListSize = 0;

GAMAdapter *GAMAdapter::Instance() {
    static GAMAdapter plumber;
    return &plumber;
}

GAMAdapter::GAMAdapter() {
    gamList = new GAMAdapterInfo[1];
}

GAMAdapter::~GAMAdapter() {
    uint32 i;
    if (gamList != NULL) {
        delete [] gamList;
    }
}

bool GAMAdapter::AddGAM(const char *gamName, const char *gamInfo, uint32 &gamIdx) {
    bool ok = true;
    //Check if a GAM with this name already exists
    GCRTemplate<GAM> gam = GetGlobalObjectDataBase()->Find(gamName);
    ok = !gam.IsValid();
    

    ConfigurationDataBase cdb;
    if (ok) {
        FString config = gamInfo;
        config.Seek(0);
        ok = cdb->ReadFromStream(config);
    }
    if (ok) {
        ok = GetGlobalObjectDataBase()->ObjectLoadSetup(cdb, NULL);
        gam = GetGlobalObjectDataBase()->Find(gamName);
    }
    if (ok) {
        ok = gam.IsValid();
    }
    
    FString ddbName;
    ddbName.Printf("%s_DDB", gamName);
    GCRTemplate<DDB> ddb = GetGlobalObjectDataBase()->Find(ddbName.Buffer());
    if (ok) {
        ok = !ddb.IsValid();
    }
    if (ok) {
        ddb = GCRTemplate<DDB>("DDB");
        ok = ddb.IsValid();
    }
    if (ok) {
        ddb->SetObjectName(ddbName.Buffer());
        ok = GetGlobalObjectDataBase()->Insert(ddb);
    }
    if (ok) {
        gamIdx = gamListSize;
        GAMAdapterInfo *gamListTemp = new GAMAdapterInfo[gamListSize + 1];
        uint32 i;
        for(i=0; i<gamListSize; i++) {
            gamListTemp[i] = gamList[i];
        }
        delete [] gamList;
        gamList = new GAMAdapterInfo[gamListSize + 1];
        for(i=0; i<gamListSize; i++) {
            gamList[i] = gamListTemp[i];
        }
        gamList[gamListSize].gamName = gamName;
        gamList[gamListSize].ddb = ddb.operator->();
        gamList[gamListSize].gam = gam.operator->();
        gamList[gamListSize].inputToGAM = NULL;
        gamList[gamListSize].outputFromGAM = NULL;
        ok = gamList[gamListSize].ddb->AddInterface(gam);
        gamListSize++;
    }
    

    return ok;
}

bool GAMAdapter::RemoveGAM(uint32 gamIdx) {
    bool ok = (gamIdx < gamListSize);
    GAM *gam = NULL;
    if (ok) {
        GetGlobalObjectDataBase()->Remove(gamList[gamIdx].gamName.Buffer());
        FString ddbName;
        ddbName.Printf("%s_DDB", gamList[gamIdx].gamName.Buffer());
        GetGlobalObjectDataBase()->Remove(ddbName.Buffer());
    }
    return ok;
}

bool GAMAdapter::AddGAMInputSignal(uint32 gamIdx, const char *gamSignalName, const char *gamSignalType) {
    bool ok = (gamIdx < gamListSize);
    GAM *gam = NULL;
    if (ok) {
        gam = gamList[gamIdx].gam;
    }
    DDB *ddb = NULL;
    if (ok) {
        ddb = gamList[gamIdx].ddb;
    }
    if (ok) {
        ok = (ddb != NULL);
    }
    if (ok) {
        ok = (gam != NULL);
    }
    DDBOutputInterface *inputToGAM;
    if (ok) {
        if (gamList[gamIdx].inputToGAM == NULL) {
            FString ownerName;
            ownerName.Printf("GAMAdapter_%s", gamList[gamIdx].gamName.Buffer());
            FString interfaceName;
            interfaceName.Printf("GAMAdapter_%s_in", gamList[gamIdx].gamName.Buffer());
            gamList[gamIdx].inputToGAM = new DDBOutputInterface(ownerName.Buffer(), interfaceName.Buffer(), DDB_WriteMode);
        }
        inputToGAM = gamList[gamIdx].inputToGAM;
    }
    if (ok) {
        printf("+++%s+++\n", gamSignalName);
        ok = inputToGAM->AddSignal(gamSignalName, gamSignalType);
    }

    return ok;
}

bool GAMAdapter::AddGAMOutputSignal(uint32 gamIdx, const char *gamSignalName, const char *gamSignalType) {
    bool ok = (gamIdx < gamListSize);
    GAM *gam = NULL;
    if (ok) {
        gam = gamList[gamIdx].gam;
    }
    DDB *ddb = NULL;
    if (ok) {
        ddb = gamList[gamIdx].ddb;
    }
    if (ok) {
        ok = (ddb != NULL);
    }
    if (ok) {
        ok = (gam != NULL);
    }
    DDBInputInterface *outputFromGAM;
    if (ok) {
        if (gamList[gamIdx].outputFromGAM == NULL) {
            FString ownerName;
            ownerName.Printf("GAMAdapter_%s", gamList[gamIdx].gamName.Buffer());
            FString interfaceName;
            interfaceName.Printf("GAMAdapter_%s_out", gamList[gamIdx].gamName.Buffer());
            gamList[gamIdx].outputFromGAM = new DDBInputInterface(ownerName.Buffer(), interfaceName.Buffer(), DDB_ReadMode);
        }
        outputFromGAM = gamList[gamIdx].outputFromGAM;
    }
    if (ok) {
        ok = outputFromGAM->AddSignal(gamSignalName, gamSignalType);
    }

    return ok;
}

bool GAMAdapter::FinaliseGAM(uint32 gamIdx, void *& inputToGAM, void *&outputFromGAM) {
    bool ok = (gamIdx < gamListSize);
    GAM *gam = NULL;
    if (ok) {
        gam = gamList[gamIdx].gam;
    }
    DDB *ddb = NULL;
    if (ok) {
        ddb = gamList[gamIdx].ddb;
    }
    if (ok) {
        ok = (ddb != NULL);
    }
    if (ok) {
        ok = (gam != NULL);
    }
    inputToGAM = NULL;
    outputFromGAM = NULL;
    if (ok) {
        if(gamList[gamIdx].inputToGAM != NULL) {
            ok = gamList[gamIdx].inputToGAM->Finalise();
            if (ok) {
                ddb->AddInterface(*gamList[gamIdx].inputToGAM);
            }
            inputToGAM = gamList[gamIdx].inputToGAM->Buffer();
        }
        if(gamList[gamIdx].outputFromGAM != NULL) {
            ok = gamList[gamIdx].outputFromGAM->Finalise();
            if (ok) {
                ddb->AddInterface(*gamList[gamIdx].outputFromGAM);
            }
            outputFromGAM = gamList[gamIdx].outputFromGAM->Buffer();
        }
    }
    if (ok) {
        ok = ddb->CheckAndAllocate();
    }
    if (ok) {
        ok = ddb->CreateLink(gam);
    }
    if (ok) {
        if(gamList[gamIdx].inputToGAM != NULL) {
            ok = ddb->CreateLink(*gamList[gamIdx].inputToGAM);
        }
    }
    if (ok) {
        if(gamList[gamIdx].outputFromGAM != NULL) {
            ok = ddb->CreateLink(*gamList[gamIdx].outputFromGAM);
        }
    }

    return ok;
}

bool GAMAdapter::LoadObjects(const char *config) {
    ConfigurationDataBase cdb;
    FString configStr = config;
    configStr.Seek(0);
    bool ok = cdb->ReadFromStream(configStr);
    if (ok) {
        ok = GetGlobalObjectDataBase()->ObjectLoadSetup(cdb, NULL);
    }
    return ok;
}

bool GAMAdapter::SendMessage(const char *destination, const char *content, unsigned int code) {
    GCRTemplate<Message> gcrtm(GCFT_Create);
    GCRTemplate<MessageEnvelope> mec(GCFT_Create);
    gcrtm->Init(code, content); 
    bool ok = mec->PrepareMessageEnvelope(gcrtm, destination);
    if (ok) {
        ok = MessageHandler::SendMessage(mec);
    }
    return ok;
}

bool GAMAdapter::ExecuteGAM(unsigned int gamIdx) {
    bool ok = (gamIdx < gamListSize);
    GAM *gam = NULL;
    if (ok) {
        gam = gamList[gamIdx].gam;
    }
    if (ok) {
        if(gamList[gamIdx].inputToGAM != NULL) {
            gamList[gamIdx].inputToGAM->Write();
        }
    }
    if (ok) {
        ok = gam->Execute(GAMOffline);
    }
    if (ok) {
        if(gamList[gamIdx].outputFromGAM != NULL) {
            gamList[gamIdx].outputFromGAM->Read();
        }
    }

    return ok; 
}
}

