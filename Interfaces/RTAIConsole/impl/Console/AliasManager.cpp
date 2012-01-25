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

#include "AliasManager.h"
#include "Common.h"
#include "File.h"

AliasManager::AliasManager()
{
	char path[256];
	FString aliaspath,abuff,line,aname,acmd;
	File af;
	Alias* tmp;
	Alias* current;
	
	aliaspath=getcwd(path,256);
	aliaspath+="/";
	aliaspath+=ALIAS_LOC;
	
	af.Open(aliaspath.Buffer());
	af.Seek(0);
	af.GetToken(abuff,"");
	af.Close();
	abuff.Seek(0);
	
	line.SetSize(0);
	aname.SetSize(0);
	acmd.SetSize(0);
	
	if (abuff.GetLine(line)){
		line.Seek(0);
		line.GetToken(aname,"=");//,NULL,"=");
		line.GetToken(acmd,"\n");
		tmp = new Alias;
		tmp->next=NULL;
		tmp->def=aname;
		tmp->cmd=acmd;
		first = tmp;
	} else {
		first=NULL;
		return;
	}
	
	line.SetSize(0);
	aname.SetSize(0);
	acmd.SetSize(0);
	current=first;
	
	while (abuff.GetLine(line)) {
		line.Seek(0);
		line.GetToken(aname,"=");//,NULL,"=");
		line.GetToken(acmd,"\n");
		tmp = new Alias;
		tmp->next=NULL;
		tmp->def=aname;
		tmp->cmd=acmd;
		current->next=tmp;
		current=tmp;
		line.SetSize(0);
		aname.SetSize(0);
		acmd.SetSize(0);
	}
}

AliasManager::~AliasManager()
{
	Alias* current=first;
	Alias* tmp=NULL;
	
	while (current!=NULL){
		tmp=current;
		current=current->next;
		delete tmp;
	}
}

FString AliasManager::CheckAlias(FString alias){
	Alias* current=first;
	FString tmp="";
	
	while (current != NULL){
		if (current->def==alias) return current->cmd;
		current=current->next;
	}
	
	return tmp;
}

FString AliasManager::DumpAliases(){
	FString tmp;
	tmp.SetSize(0);
	Alias* current=first;
	
	while (current != NULL){
		tmp+="[";
		tmp+=current->def;
		tmp+="]\t\t";
		tmp+=current->cmd;
		tmp+="\n";
		current=current->next;
	}
	return tmp;
}
