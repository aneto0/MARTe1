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
#include <stdio.h>

#include "Autocomplete.h"
#include "Directory.h"
#include "LexicalAnalyzer.h"
#include "Common.h"



FString Autocomplete(const char* tmppath,int buflength) {
	FString tmp;
	FString tmp2;
	FString Path[30];
	LA_TokenData *tk_data;
	char path[MAX_CMDLINE_DIM];
	
	LexicalAnalyzer la;
	la.AddSeparators("/");
	la.AddTerminals("\n");
	
	strncpy(path,tmppath,buflength);
	path[buflength]='\0';
	
	for (int i=0;i<30;i++) Path[i].SetSize(0);
	tmp.SetSize(0);
	tmp=path;
	
	// check if there is a leading / otherwise add current path
	char curr_path[200];
	getcwd(curr_path,200);
	
	if (path[0]!='/') { //relative path
		tmp.SetSize(0);
		tmp=curr_path;
		tmp+="/";
		tmp+=path;
	}
	tmp.Seek(0);
	
	tk_data=la.GetToken(tmp);
	int i=0;
	while(tk_data->Token()!=LATV_EOF) {
		Path[i]=tk_data->Data();
		i++;
		delete tk_data;
		tk_data=la.GetToken(tmp);
	}
        
        if(i == 0){
                i++;
                Path[i-1]="";
        }
        else if(path[strlen(path)-1]=='/') {
                Path[i-1].Seek(0);
                Path[i-1]="";	
        } else {
                tmp.SetSize(tmp.Size()-Path[i-1].Size());
        }
        
	//Now tmp contains path without last element, let's chdir	
	int res=chdir(tmp.Buffer());
	
	if (res<0) { //on error
		tmp="**ERROR**";
		return tmp;
	}
	//get back to original dir
	chdir(curr_path);
	//dir exists, create a directory object
	Directory dir(tmp.Buffer());
	int dir_dim=dir.ListSize();
	DirectoryEntry* dir_elem;
	
	int k=0;
	tmp2.SetSize(0);
	for (int j=0;j<dir_dim;j++) {
		dir_elem=(DirectoryEntry*)dir.ListPeek(j);

		if (dir_elem->IsDirectory()) {
			res=strncmp(dir_elem->Name(),Path[i-1].Buffer(),Path[i-1].Size());
			FString tmp3;
			tmp3.SetSize(0);
			tmp3=dir_elem->Name();
			if (res==0 && !(tmp3=="..") && !(tmp3==".")) {//we have at least a partial match!
				k++;
				tmp2+=tmp;
				tmp2+=dir_elem->Name();
				tmp2+="/\n";
			}
		}
	}	
	if (k==0) {
		tmp="**ERROR**";
		return tmp;
	} else if (k>1) {
		tmp="**MULTI**\n";
		tmp+=tmp2;
		return tmp;
	} else {
		return tmp2;
	}
}
