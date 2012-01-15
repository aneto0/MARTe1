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
#include "Console.h"
#include "Sleep.h"
#include "ATCAadcDrv.h"
#include "Threads.h"
#include "LoggerService.h"
#include "File.h"
#include "SXMemory.h"
#include "ConfigurationDataBase.h"
#include "GCReferenceContainer.h"
#include "GlobalObjectDataBase.h"
#include "MenuContainer.h"
#include "ProcessorType.h"

const char *cdbMap=
"+ATCA_DRIVER={\n"
"    Class = ATCAadcDrv\n"
"    Title = \"ATCA Driver test\" \n"
"    SoftTrigger=1\n"
"    NumberOfOutputs=8\n"
"    NumberOfInputs=64\n"
"    ExtTriggerAndClock=1\n"
"}\n"
"+MENUS={"
"    Class = MenuContainer\n"
"    Title = \"ATCA ADC Driver Test\" \n"
"    +START_ACK_MENU={\n"
"        Class = MenuEntry\n"
"        Title = \"Start Acquisition\"\n"
"    }\n"
"    +STOP_ACK_MENU={\n"
"        Class = MenuEntry\n"
"        Title = \"Stop Acquisition\"\n"
"    }\n"
"    +SAVE_DATA_MENU={\n"
"        Class = MenuEntry\n"
"        Title = \"Save acquired data\"\n"
"    }\n"
"    +PRINT_DATA_MENU={\n"
"        Class = MenuEntry\n"
"        Title = \"Print last buffer\"\n"
"    }\n"
"}";

volatile int keepAlive     = 0;
int64 max_timer            = 0;
float mean_exec_time       = 0;
float meansquare_exec_time = 0;
float variance_exec_time   = 0;

const int32 BUFFER_BOARD_TRF_SIZE = 33 + 2 * HEADER_LENGTH;
const int32 MAX_SAMPLES_TO_SAVE   = 100;

int32 buffer[70];
uint32 outputBuffer[3];
uint32 dataBrd1[BUFFER_BOARD_TRF_SIZE * MAX_SAMPLES_TO_SAVE];
int32 dataBrd2[BUFFER_BOARD_TRF_SIZE * MAX_SAMPLES_TO_SAVE];

int32 printLastBuffer = 0;

GCRTemplate<GenericAcqModule> driver;

void __thread_decl acquisitionThread(void *ptr){
	int   k              = 0;
	int64 timer          = 0;
	int64 timer_after    = 0;
	int   ret            = 0;
	
	max_timer            = 0;
	mean_exec_time       = 0;
	meansquare_exec_time = 0;
	variance_exec_time   = 0;
	
	driver->EnableAcquisition();

	int triggerReceived = 0;
	memset(buffer, 0, BUFFER_BOARD_TRF_SIZE * sizeof(int32));
	memset(dataBrd1, 0, BUFFER_BOARD_TRF_SIZE * MAX_SAMPLES_TO_SAVE * sizeof(int32));
	memset(dataBrd2, 0, BUFFER_BOARD_TRF_SIZE * MAX_SAMPLES_TO_SAVE * sizeof(int32));
	//Console con;
	printf("Starting\n");
	while (keepAlive != 0){
		timer = HRT::HRTCounter();
		SleepSec(10E-6);
		ret = driver->GetData(0, buffer);

		if(dataBrd1[0] == 0 && k > 5){
			triggerReceived      = 1;
			k                    = 0;
			max_timer            = 0;
			mean_exec_time       = 0;
			meansquare_exec_time = 0;
			variance_exec_time   = 0;
			printf("Trigger received!\n");

		}

		if(printLastBuffer > 0){
			printf("H[%d] = %d F[%d] = %d\n", k, 0, k, dataBrd1[BUFFER_BOARD_TRF_SIZE - 1]);
			printLastBuffer = 0;
		}

		if(triggerReceived == 1 && k < MAX_SAMPLES_TO_SAVE){
			for(int j=0; j<BUFFER_BOARD_TRF_SIZE; j++){
				dataBrd1[BUFFER_BOARD_TRF_SIZE * k + j] = buffer[j];
				dataBrd2[BUFFER_BOARD_TRF_SIZE * k + j] = buffer[BUFFER_BOARD_TRF_SIZE + j];
			}
			//dataBrd1[BUFFER_BOARD_TRF_SIZE * k] = ret * HRT::HRT::HRTPeriod() * 1000000;//buffer[0];
		}

		outputBuffer[0] = 0;
		outputBuffer[1] = 7;
		outputBuffer[2] += 10;
		driver->WriteData(0, (int32 *)outputBuffer);

		timer_after = HRT::HRTCounter();
		if ( (timer_after - timer) > max_timer) {
			max_timer = timer_after - timer;
		}
		mean_exec_time += (float)(timer_after - timer);
		meansquare_exec_time += (float)((timer_after - timer)*(timer_after - timer));
		k++;
	}
	driver->DisableAcquisition();
	mean_exec_time /= k;
	variance_exec_time = ( (meansquare_exec_time - max_timer*max_timer)/k - mean_exec_time*mean_exec_time); 
	keepAlive = 1;
}

bool StartAcquisition(StreamInterface &in,StreamInterface &out,void *userData){
	if(keepAlive != 0){
		out.Printf("Acquisition already started\n");
		return False;
	}
	keepAlive = 1;
	ProcessorType runCore(0x8);
	Threads::BeginThread(acquisitionThread, NULL, THREADS_DEFAULT_STACKSIZE, NULL, XH_NotHandled, runCore);
	return True;
}

bool StopAcquisition(StreamInterface &in,StreamInterface &out,void *userData){
        if(keepAlive == 0){
	        out.Printf("Acquisition already stopped\n");
		return False;
        }

	out.Printf("Stopping acquisition\n");
	keepAlive = 0;
	while(keepAlive != 1){
		SleepSec(1.0);
	}
	keepAlive = 0;
	out.Printf("Acquisition stopped\n");
	out.Printf("\nmax_timer = %f\n", max_timer*HRT::HRTPeriod()*1000000);
	out.Printf("mean_exec_time = %f\n", mean_exec_time*HRT::HRTPeriod()*1000000);
	out.Printf("dev_std_exec_time = %f\n", sqrt(variance_exec_time)*HRT::HRTPeriod()*1000000);
	out.Printf("max_error = %f\n", (max_timer-mean_exec_time)*HRT::HRTPeriod()*1000000);
}

bool SaveData(StreamInterface &in,StreamInterface &out,void *userData){
	out.Printf("Saving Data\n");
	File outputFile;
	FString dataDump;
	FString filename;
	FString dataDump2;
	dataDump.SetSize(BUFFER_BOARD_TRF_SIZE * MAX_SAMPLES_TO_SAVE);
	dataDump2.SetSize(BUFFER_BOARD_TRF_SIZE * MAX_SAMPLES_TO_SAVE);
	out.Printf("Printing in FString\n");
	for(int k=0; k<MAX_SAMPLES_TO_SAVE; k++){
		for(int j=0; j<BUFFER_BOARD_TRF_SIZE; j++){
			dataDump.Printf("%d ", dataBrd1[BUFFER_BOARD_TRF_SIZE * k + j]);
			dataDump2.Printf("%d ", dataBrd2[BUFFER_BOARD_TRF_SIZE * k + j]);
		}
		dataDump.Printf("\n");
		dataDump2.Printf("\n");
	}

	out.Printf("Printing in File\n");
	filename.Printf("data_%d.dump\n", time(NULL));
	outputFile.OpenNew(filename.Buffer());
	outputFile.Printf("%s", dataDump.Buffer());
	outputFile.Close();
	out.Printf("Data for board 1 saved at %s\n", filename.Buffer());
	filename.SetSize(0);
	filename.Printf("data2_%d.dump\n", time(NULL));
	outputFile.OpenNew(filename.Buffer());
	outputFile.Printf("%s", dataDump2.Buffer());	
	outputFile.Close();
	out.Printf("Data for board 2 saved at %s\n", filename.Buffer());
}

bool PrintLastBuffer(StreamInterface &in,StreamInterface &out,void *userData){
	printLastBuffer = 1;	
}

int main(int argc, char **argv) {
	
	LSSetUserAssembleErrorMessageFunction(LSAssembleErrorMessage);
	LSSetRemoteLogger("145.239.198.123",32767);
        LSStartService();

	SXMemory config((char *)cdbMap,strlen(cdbMap));
	ConfigurationDataBase cdb;
	if (!cdb->ReadFromStream(config,NULL)){
		CStaticAssertErrorCondition(ParametersError,"Init: cdb.ReadFromStream failed");
		return -1;
	}
	GCRTemplate<GCReferenceContainer> godb = GetGlobalObjectDataBase();
	godb->ObjectLoadSetup(cdb,NULL);

	GCRTemplate<MenuContainer> mc;
	mc = godb->Find("MENUS");
	if (!mc.IsValid()){
		CStaticAssertErrorCondition(FatalError,"cannot find MENUS MenuContainer object\n");
		return -2;
	}
	driver = godb->Find("ATCA_DRIVER");
	if (!driver.IsValid()){
		CStaticAssertErrorCondition(FatalError,"cannot find ATCA_DRIVER MenuContainer object\n");
		return -2;
	}
	Console con(PerformCharacterInput);
	con.SetPaging(True);

	mc->SetupItem("START_ACK_MENU", StartAcquisition, NULL, NULL, NULL);
	mc->SetupItem("STOP_ACK_MENU", StopAcquisition, NULL, NULL, NULL);
	mc->SetupItem("SAVE_DATA_MENU", SaveData, NULL, NULL, NULL);
	mc->SetupItem("PRINT_LAST_BUFFER", PrintLastBuffer, NULL, NULL, NULL);

	mc->TextMenu(con,con);
//	LSStopService();
	return 0;
}
