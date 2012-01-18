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
#include <iostream>

#include <SXMemory.h>
#include <GlobalObjectDataBase.h>
#include <ConfigurationDataBase.h>
#include <MessageServer.h>
#include <EventSem.h>

#include <Message.h>
#include <MessageEnvelope.h>
#include <MessageHandler.h>
//#include <MessageBroker.h>

#include <ConfigurableObjectProxy.h>

using namespace std;


const char *messageServerDescription =
"+MessageServer =\n"
"{\n"
"	Class      = MessageServer\n"
"	ServerPort = 9990\n"
"}";


class MessageServerRecord : public GCNamedObject
{

private:
	GCRTemplate<GCReferenceContainer> godb;

public:
	// This code goes to the init_record function
	MessageServerRecord()
	{
		// Get the global object database
		this->godb = GetGlobalObjectDataBase();

		// Create the CDB for the description of the MessageServer
		SXMemory config((char *) messageServerDescription, strlen(messageServerDescription));
		ConfigurationDataBase cdb;
		if(!cdb->ReadFromStream(config, NULL))
			cout << "Failed to load message server configuration file" << endl;
		else
			// Create the message server in the GODB
			this->godb->ObjectLoadSetup(cdb, NULL);

		// Start the message server to receive replies
		GCRTemplate<MessageServer> messageServer = godb->Find("MessageServer");
		if(!messageServer.IsValid())
		{
			cout << "You have to define a valid message server..." << endl;
			return;
		}
		if(!messageServer->Start())
		{
			cout << "Could not start the message server..." << endl;
			cout << "Not fatal but you will not be able to receive, nor handle, replies" << endl;
		}
	}

};


class MessageClientRecord : public GCNamedObject
{

private:
	GCRTemplate<GCReferenceContainer> godb;

public:
	// This code goes to the init_record function
	MessageClientRecord()
	{
		// Get the global object database
		this->godb = GetGlobalObjectDataBase();

		//
		// Create the remote object proxy
		//
		GCRTemplate<ConfigurableObjectProxy> testGam(GCFT_Create);
		testGam->SetObjectName("MyTestGam_Proxy");
		testGam->SetServer("localhost", 9988);
		testGam->SetDestinationObject("MARTe.MagneticRTTh.MyTestGam");
		testGam->SetMessageSender(testGam.operator->());
		this->godb->Insert(testGam);

		// Just list the objects to check if everything we need is there
		// Not needed on the record
		GCRCLister lister;
		this->godb->Iterate(&lister, GCFT_Recurse);
	}

	~MessageClientRecord()
	{
	}

	void process(void)
	{
		//
		// Get the remote object proxy from the GlobalObjectDataBase
		//
		GCRTemplate<ConfigurableObjectProxy> testGam = godb->Find("MyTestGam_Proxy");
		if(!testGam.IsValid())
		{
			cout << "No valid property proxy object for MyTestGam_Proxy..." << endl;
			return;
		}

		//
		// List all properties in the object
		//
		ConfigurationDataBase cdb;
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetPropertyList(cdb);

		//
		// Test value8
		//
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("value8", cdb);
		// DEBUG
		//{FString tmp; cdb->WriteToStream(tmp); cout << "value8: " << tmp.Buffer() << endl;}
		testGam->SetProperty("value8", "Value = 255\n");
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("value8", cdb);
		// DEBUG
		//{FString tmp; cdb->WriteToStream(tmp); cout << "value8: " << tmp.Buffer() << endl;}
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetPropertyInformation("value8", cdb);

		//
		// Test value8u
		//
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("value8u", cdb);
		testGam->SetProperty("value8u", "Value = 255\n");
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("value8u", cdb);
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetPropertyInformation("value8u", cdb);

		//
		// Test value16
		//
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("value16", cdb);
		testGam->SetProperty("value16", "Value = 65535\n");
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("value16", cdb);
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetPropertyInformation("value16", cdb);

		//
		// Test value16u
		//
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("value16u", cdb);
		testGam->SetProperty("value16u", "Value = 65535\n");
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("value16u", cdb);
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetPropertyInformation("value16u", cdb);

		//
		// Test value32
		//
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("value32", cdb);
		testGam->SetProperty("value32", "Value = 4294967295\n");
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("value32", cdb);
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetPropertyInformation("value32", cdb);

		//
		// Test value32u
		//
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("value32u", cdb);
		testGam->SetProperty("value32u", "Value = 4294967295\n");
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("value32u", cdb);
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetPropertyInformation("value32u", cdb);

		//
		// Test float32
		//
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("value32_f", cdb);
		testGam->SetProperty("value32_f", "Value = 1E+37\n");
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("value32_f", cdb);
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetPropertyInformation("value32_f", cdb);

		//
		// Test float64
		//
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("value64_f", cdb);
		testGam->SetProperty("value64_f", "Value = 1E+37\n");
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("value64_f", cdb);
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetPropertyInformation("value64_f", cdb);

		//
		// Test module
		//
		FString vecCdb = "";
		vecCdb += "NumberOfElements = 3\n";
		vecCdb += "Vector = { 50 100 125}\n";
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("module", cdb);
		testGam->SetProperty("module", vecCdb);
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("module", cdb);
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetPropertyInformation("module", cdb);

		//
		// Test float32 array
		//
		FString arrCdb = "";
		arrCdb += "Value = { 1 2 3 4 5 }\n";
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("array32_f", cdb);
		testGam->SetProperty("array32_f", arrCdb);
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetProperty("array32_f", cdb);
		cdb->MoveToRoot(); cdb->CleanUp();
		testGam->GetPropertyInformation("array32_f", cdb);
	}

};


/*
 * 
 */
int main(int argc, char** argv)
{
	// *************************************************************************
	// Record created by EPICS-IOC
	// *************************************************************************
	// Create the message server record (created by EPICS-IOC)
	GCRTemplate<MessageServerRecord> messageServerRecord(GCFT_Create);
	// *************************************************************************

	// *************************************************************************
	// Record created by EPICS-IOC that intends to communicate with MARTe
	// *************************************************************************
	// Create the message client record (created by EPICS-IOC)
	GCRTemplate<MessageClientRecord> messageClientRecord(GCFT_Create);

	// Process the record (called periodically by EPICS-IOC)
	messageClientRecord->process();
	// *************************************************************************

	// Wait to receive the reply
	int dummy;
	cin >> dummy;

	return 0;
}

