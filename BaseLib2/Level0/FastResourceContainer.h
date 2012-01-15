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

/**
 * @file
 * A container of resources. It will allow taking and releasing of them, in any order
 *   concurrently by any number of tasks, interrupts and processors.
 */
#ifndef _FAST_RESOURCE_CONTAINER_H
#define _FAST_RESOURCE_CONTAINER_H

#include "System.h"
#include "Atomic.h"


class FastResourceContainer{
    /** */
    uint32 treeSize;
    /** */
    uint32 treeHalf;
    /** the elements of the container */
    class FastResourceContainerData{
    public:
        /** Amount of buffer used. */
        int32 size;
        /** Locking variable, 0 if free. */
        int32 sem;
        /** */
        FastResourceContainerData(){
            size = 0;
            sem  = 0;
        }
    } *buffers;
public:
    /** */
    inline int Size(){
        if (buffers == NULL) return 0;
        return buffers[0].size;
    }

    /** */
    inline uint32 Take(){
        if (buffers[0].size == 0) return 0xFFFFFFFF;
        int pathChooser=0;
        int returnChooser=0;
        uint32 bit=treeHalf;
        uint32 pos = 0;
        while(bit<=treeHalf){
            if (bit == 0){
                if(Atomic::TestAndSet(&buffers[pos].sem)){
                    uint32 tempBit = 1;
                    int temp = pos;
                    Atomic::Decrement(&buffers[temp].size);
                    while(tempBit <= treeHalf){
                        if (temp & tempBit){
                            temp -= tempBit;
                            Atomic::Decrement(&buffers[temp].size);
                        }
                        tempBit <<=1;
                    }
                    return pos;
                } else {
                    bit=1;
                    if (pathChooser & bit) pos -= bit;
                }
            }
            // time to return one level up
            if (returnChooser & bit){

                returnChooser &= ~bit;
                bit <<= 1;
                if (pathChooser & bit)pos -= bit;
            } else { // go down
                if (pathChooser & bit){ // go right
                    pathChooser &= ~bit;
                    returnChooser |= bit; // next is up
                    int tot   = buffers[pos].size;
                    int right = 0;
                    if ((pos+bit)<treeSize) right = buffers[pos+bit].size;
                    if ((tot-right) > 0){
                        bit >>= 1;
                    }
                } else { // go left
                    pathChooser |= bit; // next is right
                    int right = 0;
                    if ((pos+bit)<treeSize) right = buffers[pos+bit].size;
                    if (right > 0){
                        pos += bit;
                        bit >>= 1;
                    }
                }
            }

        }
        pos = 0xFFFFFFFF;
        return pos;
    }

    /** */
    inline void Return(uint32 pos){
        if (pos >= treeSize) return;

        buffers[pos].sem = 0;
        Atomic::Increment(&buffers[pos].size);

        uint32 bit = 1;
        while(bit <= treeHalf){
            if (pos &bit){
                pos -=bit;
                Atomic::Increment(&buffers[pos].size);
            }
            bit <<=1;
        }
    }

    /** */
    FastResourceContainer(int nOfElements,bool taken=False){
        treeSize = nOfElements;
        treeHalf = 0;
        buffers  = NULL;

        if (treeSize < 0){
            treeSize = 0;
            return;
        }

        uint32 bit = 1;
        while(bit < treeSize) bit<<=1;
        treeHalf = bit >>1;

        buffers = new FastResourceContainerData[treeSize];
        if (!taken) for(int i=0;i<nOfElements;i++) Return(i);

    }

    /** */
    virtual ~FastResourceContainer(){
        if (buffers != NULL)  delete[] buffers;
    }


};



#endif
