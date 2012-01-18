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
#include "System.h"
#include "UDPSocket.h"
#include "FString.h"

int main(int argc, char **argv){

    uint32 port      = 14500;
    UDPSocket serverSocket; 
    if(!serverSocket.Open()){
        printf("Failed to open socket!\n");
        return -1;
    }
    if(!serverSocket.Listen(port)){
        printf("Failed to listen in port: %d\n", port);
    }
    if(!serverSocket.SetBlocking(True)){
        printf("Failed to set blocking true");
    }
    
    char    bufferRead[65536];
    uint32  readSize   = 65536;
    uint32  counter    = 0;
    uint32  totalRead  = 0;
    int64   start      = HRT::HRTCounter();
    float   bandwidth  = 0;
    int64   packetSize = 0;
    
    while(True){
        memset(bufferRead, 0, readSize);
        if(serverSocket.Read(bufferRead, readSize)){
            totalRead += readSize;
            if(packetSize == 0){
                packetSize = readSize;
            }
            if(packetSize != readSize){
                printf("Losing packets. packetSize = %d readSize = %d\n", packetSize, readSize);
            }
            if(counter % 1000 == 0){
                counter = 0;
                printf("Total read bytes: %d\n", totalRead);
                printf("Bandwitdh = %f MB/s\n", bandwidth);
                bandwidth = (totalRead / ((HRT::HRTCounter() - start) * HRT::HRTPeriod())) / 1e6;
                totalRead = 0;
                start = HRT::HRTCounter();
                
            }
            counter++;
        }
        else{
            printf("Failed reading from socket!\n");
        }
    }   
}

