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
 * An object container with the additional ability to
 * display its content on the web and to show its content in frames
 */

#if !defined (HTTP_FRAMED_RESOURCE)
#define HTTP_FRAMED_RESOURCE

#include "HttpGroupResource.h"

class HttpFramedResource;


OBJECT_DLL(HttpFramedResource)

/* flags to control HttpFramedResource */

/** divide the page vertically */
static const int32  HFRVertical = 1;

/** the names for the CDB */
static const char * HFRFlagNames[]={
    "Vertical",
    NULL
};

/** the names for the CDB */
static const int32 HFRFlagValues[]={
    HFRVertical,
    0
};


/** It is a HttpGroupResource that uses puts each resource in a frame */
class HttpFramedResource:public HttpGroupResource {
private:

    /** the amount of screen allocated to each */
    float *     weights;

    /** the size of weights*/
    int         sizeOfWeights;

    /** a set of HFRxxxx constants */
    int32       flags;
protected:
    /** */
    void CleanUp(){
        if (weights != NULL) free((void *&)weights);
        sizeOfWeights   = 0;
        flags           = 0;
    }
private:
OBJECT_DLL_STUFF(HttpFramedResource)
public:
    /** default constructor */
                            HttpFramedResource(){
        weights         = NULL;
        CleanUp();
    };

    /** with comment */
                            HttpFramedResource(
                const char *        comment){
        this->comment = comment;
        weights         = NULL;
        CleanUp();
    }

    virtual                 ~HttpFramedResource(){
        CleanUp();
    }

    /** the main entry point for HttpInterface */
    virtual     bool        ProcessHttpMessage(HttpStream &hStream);

    /**
        Parameters as specified in GCReferenceContainer and HttpInterface
        It is a container of pages which will be displayed as a single page
        The Weights vector determines the relative partitioning of the page
        Flags=Vertical allows choosing a vertical partition
        Using a class HttpFrameLink with parameter Link allows pointing to any
        page or subpage..

    */
    virtual     bool        ObjectLoadSetup(
            ConfigurationDataBase & info,
            StreamInterface *       err){

        CleanUp();
        bool ret = HttpGroupResource::ObjectLoadSetup(info,err);

        CDBExtended cdbx(info);
        int maxDim = 1;
        int size[2];
        bool ret2 = cdbx.AllocateAndReadFloatArray(weights,size,maxDim,100,"Weights");
        if (ret2){
            sizeOfWeights = size[0];

            // normalise
            int i;
            float sum = 0;
            for (i = 0;i<sizeOfWeights;i++){
                sum = sum + fabs(weights[i]);
            }
            sum = sum / 100;
            for (i = 0;i<sizeOfWeights;i++){
                weights[i] = fabs(weights[i]) / sum;
            }
        }
        cdbx.ReadFlags(flags,"Flags",HFRFlagNames,HFRFlagValues,0);

        return ret && ret2;
    }

    /** Parameters as specified in GCReferenceContainer and HttpInterface
    */
    virtual     bool        ObjectSaveSetup(
            ConfigurationDataBase & info,
            StreamInterface *       err){

        bool ret = HttpGroupResource::ObjectSaveSetup(info,err);

        CDBExtended cdbx(info);
        int maxDim = 1;
        int size[2];
        size[0] = sizeOfWeights;
        ret = ret && cdbx.WriteFloatArray(weights,size,maxDim,"Weights");
        return ret;
    }

};



#endif


