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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <rtai_nam2num.h>

#include "fcomm_ipc.h"
#include "api.h"

void fileChecker(void *args)
{
    char *fname = (char *)args;
    remove(fname);
    FILE* file;
    while(!(file = fopen(fname, "r")))
    {
        sleep(5);        
    }
    
    printf("File found... exiting\n");            
    fclose(file);        
}

int main(int argc, char** argv) 
{
#ifdef _QUIT_FROM_KEYBOARD_    
    char read[256];
    char *tok;
    int pid = 0;
#endif    
    
    if(argc == 4){
        printf("Going to use verbose mode with parameters: %s %d %d\n", argv[1], atoi(argv[2]), atoi(argv[3]));
        initDebugSocket(argv[1], atoi(argv[2]), atoi(argv[3]));
    }
    int ret = init_shm();
    if(ret < 0) 
    {
        printf("Error initiating...%d\n", ret);
        clean_close_shm();
        exit(-1);
    }
    else
    {
        printf("Init OK!\n");
        clear_screen();
    }
        
    fflush(stdin);

#ifdef _QUIT_FROM_KEYBOARD_    
    while(strcmp(read, "!quit!\n") != 0)
    {
        //textcolor(BRIGHT, GREEN, BLACK);
        printf("[fusion-rt]#");
        //textcolor(RESET, WHITE, BLACK);
        fgets(read, 256, stdin);
        if(strncmp(read, "_", 1) == 0)
        {
            tok = strtok(read, "_");

            pid = fork();
            if(pid == 0)
            {
                execl("/bin/sh", "sh", "-c", tok, NULL);
                return 0;
            }
            continue;                
        }        
    }
#endif
    //TODO: commented out as it conflicts with the above defs for argv. Brutal solution!
//    if(argc < 2)
        fileChecker("/tmp/exit");
//    else
//        fileChecker(argv[1]);
    
    clean_close_shm();
    closeDebugSocket();
    return 0;
}

