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
 * $Id: CStream.cpp 3 2012-01-15 16:26:07Z aneto $
 *
**/

extern "C"{

    bool GetCStringToken(const char *&input,char *buffer,const char *terminator,uint32 maxSize);

    char *DestructiveGetCStringToken(char *&input,const char *terminator,char *saveTerminator=NULL,const char *skip="");

}


    /** extract a token from the string into a string until a terminator or 0 is found.
        The maximum string size is maxSize -1
        Skips any number of leading terminators
        returns true if some data was read. False only on no data available
        The input pointer will be left pointing at the terminator. The terminator is not consumed    */
    bool  GetCStringToken(
                                const char *&       input,
                                char *              buffer,
                                const char *        terminator,
                                uint32              maxSize);

    /** extract a token from the string into a string until a terminator or 0 is found.
        Skips any number of leading characters that are skipCharacters and terminator!
        affects the input by placing 0 at the end of each token
        Never returs NULL unless the input is NULL;
        The terminator (just the first encountered) is consumed in the process and saved in saveTerminator if provided
    */
    char *DestructiveGetCStringToken(
                                char *&             input,
                                const char *        terminator,
                                char *              saveTerminator  =NULL,
                                const char *        skipCharacters  ="");


#endif



/*************************************************************/
//
//              MOVE TO C FILE
// 
/************************************************************/


bool GetCStringToken(const char *&input,char *buffer,const char *terminator,uint32 maxSize){
    maxSize--; // for the trailing 0
    char *p = buffer;
    while(maxSize > 0){
        if (*input == 0){
            *p = 0;
            if (p == buffer) return False;
            else             return True;
        }
        char c = *input;
        if ((strchr(terminator,c)!=NULL)||(c==0)){
            // exit only if some data was read, otw just skip separator block
            if (p != buffer){
                *p = 0;
                return True;
            }
        } else {
            *p++ = c;
            maxSize--;
        }
        input++;
    }
    *p = 0;
    return True;
}

char *DestructiveGetCStringToken(char *&input,const char *terminator,char *saveTerminator,const char *skip){
    if (skip == NULL) skip = terminator;
    char *p = input;
    if (p == NULL) return NULL;
    while(1){
        if (*input == 0){
            if (saveTerminator!=NULL) *saveTerminator = *input;
            return p;
        }

        char c = *input;
        bool isTerminator = (strchr(terminator,c)!=NULL);
        bool isSkip       = (strchr(skip      ,c)!=NULL);

        if (isTerminator || (c == 0)){
            // exit only if some data was read, otw just skip separator block
            if ((p != input) || (!isSkip)){

                if (saveTerminator!=NULL) *saveTerminator = c;

                *input++ = 0;
                return p;
            } else  p++; // skip separator
        }
        input++;
    }
    return NULL;
}




