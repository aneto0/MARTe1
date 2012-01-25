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
#include "VarManager.h"

VarManager::VarManager()
{
	first=NULL;
}

VarManager::~VarManager()
{
	Variable* tmp=first;
	
	while (tmp!=NULL){
		Variable* tmp2=tmp;
		tmp=tmp->next;
		delete tmp2;
	}
}


bool VarManager::AddVariable(FString Name, int Data){
	// Check if variable already exists
	bool res=false;
	Variable* tmp=first;
	
	while (!res && tmp!=NULL){
		if (tmp->Name==Name) res=true;
		if (!res) tmp=tmp->next;
	}
	// If variable already exists, update it's value and exit
	if (res==true){
		tmp->Data=Data;
		return true;
	}
	
	tmp=new Variable;
	tmp->Name=Name;
	tmp->Data=Data;
	tmp->next=first;
	first=tmp;
	return true;
}


bool VarManager::DelVariable(const char* Name){
	Variable* tmp=first;
	Variable* tmp2=NULL;
	
	// If the variable is at the beginning of the list...
	if (first->Name==Name) {
		if (first->next==NULL){ // Only one element
			delete first;
			first=NULL;
			return true;
		} else { // otherwise delete first and update first pointer
			first=first->next;
			delete tmp;
			return true;
		}
	}
	
	// ...otherwise search the variable and delete it
	tmp=first->next;
	tmp2=first;
	while (tmp!=NULL){
		if (tmp->Name==Name){
			tmp2->next=tmp->next;
			delete tmp;
			return true;
		}
		tmp2=tmp;
		tmp=tmp->next;
	}
	return false;
}




Variable* VarManager::SeekVariable(const char* Name){
	Variable* tmp=first;
	
	while (tmp!=NULL){
		if (tmp->Name==Name) return tmp;
		tmp=tmp->next;
	}
	return NULL;
}

Variable* VarManager::ReturnFirst(){
	return first;
}

