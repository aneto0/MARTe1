/**
 * @file MDSWriterNode.h
 * @brief Header file for class MDSWriterNode
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

#ifndef MDS_WRITER_NODE
#define MDS_WRITER_NODE

#include "mdsobjects.h"
#include "GCNamedObject.h"
#include "FString.h"

/**
 * @brief Class which describes an MDSplus node.
 */
OBJECT_DLL(MDSWriterNode)
class MDSWriterNode : public GCNamedObject {
OBJECT_DLL_STUFF(MDSWriterNode)
public:
    /**
     * @brief NOOP
     */
    MDSWriterNode();

    /**
     * @brief frees all the memory allocated by this node
     */
    ~MDSWriterNode();

    /**
     * @brief Create the TreeNode in specified tree
     * @param tree a valid MDSplus::Tree
     * @return false if the node could not be allocated
     */
    bool AllocateTreeNode(MDSplus::Tree *tree);

    /**
     * @brief Write data into the node.
     * @return false if the data could not be written
     */
    bool Write(void *data);

    /**
     * @brief Loads all the configuration parameters. AllocateTreeNode must
     * be subsequentially called by the Tree owner
     */
    virtual bool ObjectLoadSetup(ConfigurationDataBase &cdb, StreamInterface *err);

    /**
     * @return the number of Words allocated by this node (i.e. numberOfSamples * sizeof(nodeType) / sizeof(int32))
     */
    uint32 GetNumberOfWords();

private:
    /**
     * The name of the MDSplus node
     */
    FString nodeName;

    /**
     * The node type (as defined in mdsobjects.h)
     */
    uint32 nodeType;

    /**
     * Number of samples to be stored on each write operation
     */
    int32 numberOfSamples;

    /**
     * Period between samples
     */
    double period;

    /**
     * Phase shift (in samples) of the first sample
     */
    int32 phaseShift;

    /**
     * The MDSplus tree node where data is stored
     */
    MDSplus::TreeNode *node;

    /**
     * Number of times Write was called
     */
    uint64 nOfWriteCalls;

    /**
     * True if a decimated signal is also to be stored using makeSegmentMinMax
     */
    bool decimatedMinMax;
    
    /**
     * The decimated node for makeSegmentMinMax
     */
    FString decimatedNodeName;
    MDSplus::TreeNode *decimatedNode;

    /**
     * the number of Words allocated by this node (i.e. numberOfSamples * sizeof(nodeType) / sizeof(int32))
     */
    uint32 numberOfWords;

    /**
     * Data is stored in this buffer before triggering a makeSegment/makeSegmentMinMax
     */
    char *bufferedData;
    uint32 currentBuffer;
    int32 makeSegmentAfterNWrites;
};

#endif

