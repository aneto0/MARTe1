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
#include <stdio.h>
#include <stdlib.h>
#include "SrTrATCADrv.h"
#include "CDBExtended.h"

int main(int argc, char **argv){
    SrTrATCADrv driver;
    CDBExtended cdb;
    File f;
    f.OpenRead("/home/jet/kn3g-3.00/StartUp/MARTe-kn3g-storage-no-rt.cfg");
    cdb->ReadFromStream(f);
    f.Close();
    cdb->Move("+MARTe.+DriverPool.+SrTrATCASlot15");
    driver.ObjectLoadSetup(cdb, NULL);
    //Put these public in the driver if you want to test
    driver.StartAcquisition();
    SleepSec(1.0);
    driver.PulseStart();
    SleepSec(5.0);
    driver.StopAcquisition();
    driver.StoreRamData();
    return 0;
}

