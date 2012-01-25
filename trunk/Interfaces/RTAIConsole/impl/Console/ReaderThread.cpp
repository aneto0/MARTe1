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
#include <sys/select.h>

#include "Console.h"
#include "Threads.h"

#include "Common.h"
#include "ReaderThread.h"

int wait_on_select(int fd){
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(fd, &read_set);
        int ret = select(fd + 1, &read_set, NULL, NULL, NULL);
        return ret;
}

void __thread_decl ReaderThread(void *data){
	Console *con=(Console *)data;

	char buffer[16384];
    memset(buffer, 0, 16384);
    int fd = open(RTAICOUT, O_RDWR);
    fd_set read_set;
    int res = 0;
    FD_ZERO(&read_set);
    FD_SET(fd, &read_set);
    while(1){
    	wait_on_select(fd);
        read(fd, buffer, 16384);
        con->Printf("%s", buffer);
        memset(buffer, 0, 16384);
    }
}

bool CheckFifo() {
	int fd = open(RTAICOUT, O_RDWR);
	int tmp = open(RTAICIN, O_RDWR);
	
	if (fd<0 || tmp<0) { //at least one fifo doesn't exist
		return false;
	} else {
		close(tmp);
		return true;
	}
}
