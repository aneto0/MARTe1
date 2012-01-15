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
 * parses a name with segments like paths or structure naming 
 */
#if !defined(SEGMENTED_NAME)
#define SEGMENTED_NAME

#include "System.h"
#include "StreamInterface.h"

#define SN_GRANULARITY 16

class SegmentedName{
    /** 0 separated names */
    char **names;
    /** index in the separators list provided at object construction */
    int  *separators;
    /** */
    int numberOfSegments;
    /** */
    int maxNumberOfSegments;
    /** similar to strcmp but limit search to size of separator */
    bool Compare(const char *&name,const char *separator){
        while ((*name != 0) && (*separator != 0) && (*name == *separator)){
            name++;
            separator++;
        }
        return (*separator == 0);
    }

    /** */
    void CleanUp(){
        int i = 0;
        for (i = 0;i < numberOfSegments;i++){
            if (names[i]) free((void *&)names[i]);
        }
        if (names) free((void *&)names);
        if (separators) free((void *&)separators);
        maxNumberOfSegments = 0;
        numberOfSegments    = 0;
    }

public:

    /** */
    SegmentedName(){
        maxNumberOfSegments = 0;
        numberOfSegments    = 0;
        names               = NULL;
        separators          = NULL;
    }

    /** separators is a N&ULL terminated vector of separators strings */
    SegmentedName(const char *name,const char **separatorNames){
        maxNumberOfSegments = 0;
        numberOfSegments    = 0;
        names               = NULL;
        separators          = NULL;
        if (name == NULL) return;
        if (strlen(name) == 0) return;

        const char *start = name;
        while (name[0] != 0){
            const char **seps=separatorNames;
            const char *nam=name;
            int sepNo=0;
            // try each separator
            while((seps[sepNo] != NULL) && (!Compare(nam,seps[sepNo]))){
                seps++;
                nam = name;
            }
            if (seps[sepNo] != NULL){
                // from start to begin of separator
                int size = name - start;
                AddSegment(start,size,sepNo);
                // move start point to end of separator
                start = nam;
                // keep on searching after
                name  = nam;
            } else {
                // next
                name++;
            }
        }
        int size = name - start;
        if (size > 0)
            AddSegment(start,size,-1);
    }

    /** */
    ~SegmentedName(){
        CleanUp();
    }
    /** */
    const char *Name(int index)const{
        if (index >= numberOfSegments) return NULL;
        if (index < 0) return NULL;
        return  names[index];
    }
    /** */
    const char *operator[](int index)const {
        if (index >= numberOfSegments) return NULL;
        if (index < 0) return NULL;
        return  names[index];
    }
    /** */
    int Separator(int index){
        if (index >= (numberOfSegments)) return -1;
        return  separators[index];
    }
    /** */
    int NumberOfSegments() const{
        return numberOfSegments;
    }
    /** */
    void operator=(SegmentedName &sname){
        int i;
        for (i=0;i<sname.NumberOfSegments();i++){
            AddSegment(sname.Name(i),strlen(sname.Name(i)),Separator(i));
        }
    }

    /** size characters of name are copied */
    void AddSegment(const char *name,int size,int separator){
        // this means that we used up to the last one
        if (numberOfSegments == maxNumberOfSegments){
            // allocate larger ones
            char **newNames      = (char **)malloc(sizeof(char *)*(maxNumberOfSegments+SN_GRANULARITY));
            int   *newSeparators = (int *)  malloc(sizeof(int)   *(maxNumberOfSegments+SN_GRANULARITY));
            // copy old content
            int i;
            for (i=0;i<maxNumberOfSegments;i++){
                newNames[i] = names[i];
                newSeparators[i] = separators[i];
            }
            // zero extra space
            for (;i<(maxNumberOfSegments+SN_GRANULARITY);i++){
                newNames[i] = NULL;
                newSeparators[i] = -1;
            }
            // throw away old containers
            if (names)free((void *&)names);
            if (separators)free((void *&)separators);
            // copy over new ones
            names = newNames;
            separators = newSeparators;
            // update container size
            maxNumberOfSegments+=SN_GRANULARITY;
        }
        // finally add new segment
        names[numberOfSegments] = (char *)malloc((size+1)*sizeof(char));
        strncpy(names[numberOfSegments],name,size);
        names[numberOfSegments][size]=0;
        separators[numberOfSegments] = separator;
        numberOfSegments++;
    }
    /** */
    void RemoveLastSegment(){
        if (numberOfSegments == 0) return;
        numberOfSegments--;
        if (names[numberOfSegments])
            free((void *&)names[numberOfSegments]);
    }
    /** */
    void SaveToStream(StreamInterface &stream,const char **separatorNames=NULL,int fromSegment=0){
        const char *defSep[] = { ".",NULL};
        int nOfSeparators = 0;
        if (separatorNames != NULL){
            while (separatorNames[nOfSeparators] != NULL) nOfSeparators++;
        } else {
            nOfSeparators = 1;
            separatorNames= defSep;
        }
        if (numberOfSegments > fromSegment) stream.Printf("%s",Name(fromSegment));
        for (int i = fromSegment+1; i < numberOfSegments; i++){
            int sepNo = Separator(i-1);
            if (sepNo > nOfSeparators) sepNo = nOfSeparators-1;
            stream.Printf("%s%s",separatorNames[sepNo],Name(i));
        }

    }

};

#endif
