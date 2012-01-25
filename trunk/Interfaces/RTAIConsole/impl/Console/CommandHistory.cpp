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
#include <unistd.h>

#include "File.h"
#include "Common.h"
#include "CommandHistory.h"


CommandHistory::CommandHistory() {
	char path[256];
	bool res;
	
	HistPath=getcwd(path,256);
	HistPath+="/";
	HistPath+=HIST_LOC;
	
	first=NULL;
	last=NULL;
	current=NULL;
	list_end=true;
	HistoryCount=0;
	
	res=LoadHistory();
}



CommandHistory::~CommandHistory() {
	bool res;
	HistoryElement* tmp;
	
	res=FlushFile();
	
	// Destroy everything
	while (first!=NULL){
		tmp=first;
		first=tmp->next;
		delete tmp;
	}
}


bool CommandHistory::FlushFile() {
	HistoryElement* tmp;
	
	
	HistFile.OpenNew(HistPath.Buffer());
	HistFile.SetSize(0);
	tmp=first;
	
	while (tmp!=NULL){
		HistFile.Printf("%s\n",tmp->Command.Buffer());
		tmp=tmp->next;
	}
	HistFile.Close();
}




bool CommandHistory::LoadHistory() {
	FString CmdLine;
	FString HistBuf;
	HistoryElement* tmp;
	
	// Buffering history file
	HistFile.Open(HistPath.Buffer());
	HistFile.Seek(0);
	HistFile.GetToken(HistBuf,"");
	HistFile.Close();
	HistBuf.Seek(0);
	
	// Seek the first element
	if (HistBuf.GetLine(CmdLine)) {
		tmp = new HistoryElement;
		tmp->prev=NULL;
		tmp->next=NULL;
		tmp->Command=CmdLine;
		first=tmp;
		last=tmp;
		HistoryCount++;
	}else{ //error, file is void!
		return false;
	}
	CmdLine.SetSize(0);
	
	// Add elements
	
	while(HistBuf.GetLine(CmdLine)){
		InsertCommand(CmdLine);
		CmdLine.SetSize(0);
	}
	return true;
}





void CommandHistory::InsertCommand(FString CmdLine) {
	HistoryElement* tmp;
	
	// Check to avoid adding the same command over and over
	if ( (last!=NULL) && (last->Command==CmdLine) ) return;
	
	// Check if we have too many commands
	if (HistoryCount>=HIST_MAX_LENGTH) { // Move the list one down
		//Move first element to second and delete first
		tmp=first;
		first=first->next;
		delete tmp;
		HistoryCount--;
		//update pointers
		first->prev=NULL;
		
	}
	
	tmp = new HistoryElement;
	tmp->Command=CmdLine;
	
	tmp->prev=last;
	tmp->next=NULL;
	
	if (last!=NULL) { //exists a last element, i.e. list not empty
		last->next=tmp;
	}else{ //list empty, initialize first element also
		first=tmp;
	}
	last=tmp;
	current=tmp;
	HistoryCount++;
}


void CommandHistory::ResetCurrent() {
	current=last;
}


FString CommandHistory::GetNext() {
	FString tmp;
	tmp="";
	if (current==NULL) return tmp;
	if (current->next!=NULL){
		current=current->next;
		return current->Command;
	}
	list_end=true;
	return tmp;
}

FString CommandHistory::GetPrev() {
	FString tmp;
	tmp="";
	if (current==NULL) return tmp;
	if (current->prev!=NULL) {
		if (list_end==false){
			current=current->prev;
			return current->Command;
		}else{
			list_end=false;
			//current=current->prev;
			return current->Command;
		}
	}
	return current->Command;
}
