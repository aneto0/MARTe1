/**
 * @file MDSWriterNode.cpp
 * @brief Source file for class MDSWriterNode
 * @date 06/02/2016
 * @author Andre' Neto
 *
 * @copyright Copyright 2015 F4E | European Joint Undertaking for ITER and
 * the Development of Fusion Energy ('Fusion for Energy').
 * Licensed under the EUPL, Version 1.1 or - as soon they will be approved
 * by the European Commission - subsequent versions of the EUPL (the "Licence")
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence at: http://ec.europa.eu/idabc/eupl
 *
 * @warning Unless required by applicable law or agreed to in writing, 
 * software distributed under the Licence is distributed on an "AS IS"
 * basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the Licence permissions and limitations under the Licence.
 */

#include "MDSWriterNode.h"
#include "ConfigurationDataBase.h"
#include "CDBExtended.h"

MDSWriterNode::MDSWriterNode(){
    node = NULL;
    decimatedNode = NULL;
    period = 1;
    numberOfSamples = 0;
    phaseShift = 0;
    nOfWriteCalls = 0;
    nodeType = 0;
    numberOfWords = 0;
    decimatedMinMax = false;
    char *bufferedData = 0;
    currentBuffer = 0;
    makeSegmentAfterNWrites = 0;
    minMaxResampleFactor = 100;
}

MDSWriterNode::~MDSWriterNode(){
    if(node != NULL){
        //TODO check if the node should be deleted, or if this is done by the tree...
        //delete node;
    }    
    if(decimatedNode != NULL){
        //TODO check if the node should be deleted, or if this is done by the tree...
        //delete node;
    } 
    if(bufferedData != NULL){
        free((void *&)bufferedData);
    }
}

bool MDSWriterNode::ObjectLoadSetup(ConfigurationDataBase &config, StreamInterface *err){
    GCNamedObject::ObjectLoadSetup(config, err); 
    bool ok = true;
    CDBExtended cdb(config);
    if(!cdb.ReadFString(nodeName, "NodeName")){
        AssertErrorCondition(FatalError, "MDSWriterNode::ObjectLoadSetup: %s no node name specified for signal.", Name());
        ok = false;
    }
    cdb.ReadFString(decimatedNodeName, "DecimatedNodeName", "");
    decimatedMinMax = (decimatedNodeName.Size() > 0);

    FString typeOfData;
    if(!cdb.ReadFString(typeOfData, "NodeType", "")){
        AssertErrorCondition(FatalError, "MDSWriterNode::ObjectLoadSetup: %s NodeType not specified for node with name %s", Name(), nodeName.Buffer());
        ok = false;
    }
	if(typeOfData == "int32"){
        nodeType = DTYPE_L;
    }
    else if(typeOfData == "int16"){
        nodeType = DTYPE_W;
    }
    else if(typeOfData == "int64"){
        nodeType = DTYPE_Q;
    }
    else if(typeOfData == "float"){
        nodeType = DTYPE_FLOAT;
    }
    else if(typeOfData == "double"){
        nodeType = DTYPE_DOUBLE;
    }
    else {
        AssertErrorCondition(FatalError, "MDSWriterNode::ObjectLoadSetup: %s NodeType %s not supported for node with name %s", Name(), typeOfData.Buffer(), nodeName.Buffer());
        ok = false;
    } 

    if(!cdb.ReadInt32(numberOfSamples, "NumberOfSamples", 1)){
        AssertErrorCondition(Warning, "MDSWriterNode::ObjectLoadSetup: %s NumberOfSamples not specified for node with name %s. Using default: %d", Name(), nodeName.Buffer(), numberOfSamples);
    }
    if(!cdb.ReadDouble(period, "Period", 1)){
        AssertErrorCondition(Warning, "MDSWriterNode::ObjectLoadSetup: %s UsecPeriod not specified for node with name %s. Using default: %llf", Name(), nodeName.Buffer(), period);
    }
    if(!cdb.ReadInt32(phaseShift, "SamplePhase", 0)){
        AssertErrorCondition(Warning, "MDSWriterNode::ObjectLoadSetup: %s SamplePhase not specified for node with name %s. Using default: %d", Name(), nodeName.Buffer(), phaseShift);
    }
    cdb.ReadInt32(makeSegmentAfterNWrites, "MakeSegmentAfterNWrites", 1);
    cdb.ReadInt32(minMaxResampleFactor, "MinMaxResampleFactor", 100);

    uint32 typeMultiplier = 0;
    
    if (nodeType == DTYPE_L){
        typeMultiplier = 1;
    }
    else if (nodeType == DTYPE_W){
        typeMultiplier = 1;
    }
    else if (nodeType == DTYPE_Q){
        typeMultiplier = sizeof(uint64)/sizeof(uint32);
    }
    else if (nodeType == DTYPE_FLOAT){
        typeMultiplier = sizeof(float)/sizeof(uint32);
    }
    else if (nodeType == DTYPE_DOUBLE){
        typeMultiplier = sizeof(double)/sizeof(uint32);
    }
    numberOfWords = numberOfSamples * typeMultiplier;
    if(makeSegmentAfterNWrites > 0){
        bufferedData = (char *)malloc(makeSegmentAfterNWrites * numberOfWords * sizeof(int32));
    }
    else{
        AssertErrorCondition(Warning, "MDSWriterNode::ObjectLoadSetup: %s makeSegmentAfterNWrites must be > 0 (instead is %d)", Name(), makeSegmentAfterNWrites);
    }

    //Note that we have to multiply by two because the minimum size is one word (sizeof(int32))
    if (nodeType == DTYPE_W){
        numberOfSamples *= 2;
    }
    numberOfSamples *= makeSegmentAfterNWrites;
    return ok; 
}
bool MDSWriterNode::AllocateTreeNode(MDSplus::Tree *tree){
    bool ok = true;
    try {
        node = tree->getNode(nodeName.Buffer());	
        node->deleteData();
        if(decimatedMinMax) {
            decimatedNode = tree->getNode(decimatedNodeName.Buffer());
            decimatedNode->deleteData();
        }
    }
    catch(MDSplus::MdsException &exc){
        AssertErrorCondition(FatalError, "MDSWriterNode::ObjectLoadSetup: %s Failed opening node with name %s", Name(), nodeName.Buffer());
        ok = false;
    }
    return ok;
}

bool MDSWriterNode::Write(void *data){
    if(currentBuffer < makeSegmentAfterNWrites){
        memcpy(bufferedData + (currentBuffer * numberOfWords * sizeof(int32)), data, numberOfWords * sizeof(int32));
        currentBuffer++;
    }
    if(currentBuffer == (makeSegmentAfterNWrites - 1)){
        currentBuffer = 0;
        double start = nOfWriteCalls * numberOfSamples * period;
        start += phaseShift * period;
        double end = start + ((numberOfSamples - 1) * period);
        MDSplus::Data *startD =  new MDSplus::Float64(start);
        MDSplus::Data *endD =  new MDSplus::Float64(end);
        MDSplus::Data *dimension = new MDSplus::Range(startD, endD, new MDSplus::Float64(period));
        MDSplus::Array *array = NULL;
       
        if (nodeType == DTYPE_W){
            array = new MDSplus::Int16Array((int16 *)bufferedData, numberOfSamples);
        }
        else if(nodeType == DTYPE_L){
            array = new MDSplus::Int32Array(((int32 *)bufferedData), numberOfSamples);
        }
        else if (nodeType == DTYPE_Q){
            array = new MDSplus::Int64Array((int64_t *)bufferedData, numberOfSamples);
        }
        else if (nodeType == DTYPE_FLOAT){
            array = new MDSplus::Float32Array((float *)bufferedData, numberOfSamples);
        }
        else if (nodeType == DTYPE_DOUBLE){
            array = new MDSplus::Float64Array((double *)bufferedData, numberOfSamples);
        }
        if(array != NULL){
            if(decimatedMinMax){
                node->makeSegmentMinMax(startD, endD, dimension, array, decimatedNode, minMaxResampleFactor);
            }
            else{
                node->makeSegment(startD, endD, dimension, array);
            }
            MDSplus::deleteData(array);
        }
        MDSplus::deleteData(dimension);
        nOfWriteCalls++;
    }
    return true;
}

uint32 MDSWriterNode::GetNumberOfWords(){
    return numberOfWords;
}

OBJECTLOADREGISTER(MDSWriterNode,"$Id: MDSWriterNode.cpp $")

