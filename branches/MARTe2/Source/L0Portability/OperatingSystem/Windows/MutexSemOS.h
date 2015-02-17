/*
 * Copyright 2015 F4E | European Joint Undertaking for 
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
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
 * See the Licence  
   permissions and limitations under the Licence. 
 *
 * $Id: Endianity.h 3 2012-01-15 16:26:07Z aneto $
 *
**/
/**
 *  @file 
 *  Mutex semafore
 */

#ifndef MUTEX_SEM_OS_H
#define MUTEX_SEM_OS_H

#include "../../TimeoutType.h"

/** open the semafore with a given initial state */
bool MutexSemOSCreate(HANDLE semH, bool locked){
	semH = CreateMutex(NULL,(locked==True),NULL);
	return (semH!=NULL);
}

/** close the semafore handle */
bool MutexSemOSClose(HANDLE semH){
	if (semH==(HANDLE)NULL) return True;
	if( CloseHandle(semH)==FALSE){
		return False;
	}
	semH=NULL;
	return True;
}

/** grab the semafore */
bool MutexSemOSLock(HANDLE semH, TimeoutType msecTimeout){
	DWORD ret = WaitForSingleObject(semH,msecTimeout.msecTimeout);
	if (ret == WAIT_FAILED){
		return False;
	}
	if (ret == WAIT_TIMEOUT) return False;
	return True;
}

/** returns the ownership */
bool MutexSemOSUnLock(HANDLE semH){
	if(ReleaseMutex(semH)==FALSE){
		return False;
	}
	return True;
}

/** locks without wasting time */
inline bool MutexSemOSFastLock(HANDLE semH, TimeoutType msecTimeout){
	int ret = WaitForSingleObject(semH,msecTimeout.msecTimeout);
	return ((ret!=(int)WAIT_FAILED) &&(ret!=(int)WAIT_TIMEOUT));
}


/** unlock semafore fast */
inline bool MutexSemOSFastUnLock(HANDLE semH){
	return (ReleaseMutex(semH)==TRUE);
}

/** just try to lock it returning immediately */
inline bool MutexSemOSFastTryLock(HANDLE semH){
	int ret = WaitForSingleObject(semH,0);
	return ((ret!=(int)WAIT_FAILED) &&(ret!=(int)WAIT_TIMEOUT));
}

#endif

