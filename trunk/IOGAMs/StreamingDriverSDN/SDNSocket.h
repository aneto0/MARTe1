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
/*************************************************************************
#
# Project:        CODAC FPSC-beta
#
# File:              devMARTeSDNFelix-support.cpp
#
# Description: Class implementation to create receiver and publisher SDN Object
#                     Hybrid implementation to MARTe and EPICS
#
# version:       1.0
#
# Author:         Bruno Santos (IPFN-Portugal)
#
# Copyright (c): 2011 ITER Organization,
#                 CS 90 046
#                 13067 St. Paul-lez-Durance Cedex
#                 France
#
#*************************************************************************/

/* 
 * File:   SDNSocket.h
 * Author: bsantos
 *
 * Created on May 6, 2011, 1:54 PM
 */

#ifndef SDNSOCKET_H
#define	SDNSOCKET_H

#include <sdn.h>
//#include <tcn.h>
#ifdef _MARTE
#include "Object.h"
#else
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#endif

//Class to create SDN API Object
#ifdef _MARTE
//Descend from "public Object" to provide MARTe VAssertErrorCondition calls
class SDNSocket : public Object {
#else
class SDNSocket {
#endif
    
protected:
    //IP address destination in publisher [socket]
    char address[20];
    //Listen port in reveiver and destination port in publisher [socket]
    int port;
    //Destination nodes list in publisher [dolphin]
    char nodes[20];
    //Node ID in receiver and publisher [dolphin]
    int remoteNode;
    //Segment ID in receiver and publisher [shmem]

    char segmentID[5];  
    //SDN API data block
    sdn_block_t block;    
    //Block size requested in receiver and publisher
    unsigned int segmentSize;
    //Header size
    unsigned int headerSize;
    //Block size assigned in receiver and publisher
    unsigned int segmentSizeConf;

    //Connection timeout on connect block
    unsigned int connectTimeout;
    //Wait timeout on message receiveing
    int waitTimeout;

   public:
        //Global Constructor
        SDNSocket (){
	    strcpy(address, "224.0.1.10");
	    port = 50000;

	    strcpy(nodes, "4,8,12");
	    remoteNode = 4;    

	    strcpy(segmentID, "1");

	    block = NULL;

	    headerSize = 24;	    

	    segmentSize = 0;
	    
	    segmentSizeConf = 0;
	    
	    connectTimeout = 0;
	    
	    waitTimeout = 0;

	    //init sdn configurations
	    // Initialize TCN
	    int err;
/*	    error = tcn_init();
	    if (error != TCN_SUCCESS)
	    {
//		printf("ERROR: cannot initialize TCN\n [%d] -> [%s]\n", tcn_strerror(error));	
	        printFatalError("ERROR: cannot initialize TCN\n");
//	        return;
	    }
*/
	    // Initialize SDN
	    err = sdn_init();
/*	    if (err != SDN_SUCCESS && err != 22)
	    {
		printf("ERROR: cannot initialize SDN [%d] -> [%s]\n", err, sdn_strerror(err));
	        printFatalError("ERROR: cannot initialize SDN [%d] -> [%s]\n", err, sdn_strerror(err));
//	        return;
	    }*/

	}

        //Global Destructor
        ~SDNSocket (){};

	//set configurable parameters
        void setAddress(char * _address){
	    strcpy(this->address, _address);
	}

        void setNodes(char * _nodes){
	    strcpy(this->nodes, _nodes);
	}

        void setSegmentID(char * _segmentID){
	    strcpy(this->segmentID, _segmentID);
	}

        void setPort(int _port){
	    this->port = _port;
	}

        void setRemoteNode(int _remoteNode){
	    this->remoteNode = _remoteNode;
	}

    void setHeaderSize(int _header_size){
	    this->headerSize = _header_size;
	}

        void setSegmentSize(int _segment_size){
	    this->segmentSize = _segment_size;
	}

        void setConnectTimeout(unsigned int _connectTimeout){
	    this->connectTimeout = _connectTimeout;
	}

        void setWaitTimeout(unsigned int _waitTimeout){
	    this->waitTimeout = _waitTimeout;
	}

	//get configurable parameters        
        char * getAddress(){
	    return address;
	}

        char * getNodes(){
	    return nodes;
	}

        char * getSegmentID(){
	    return segmentID;
	}

        int getPort(){
	    return port;
	}

        int getRemoteNode(){
	    return remoteNode;
	}

    int getHeaderSize(){
	    return headerSize;
	}

        int getSegmentSizeRequired(){
	    return segmentSize;
	}

        int getSegmentSizeConfigured(){
	    return segmentSizeConf;
	}     

        int getConnectTimeout(){
	    return connectTimeout;
	} 

        int getWaitTimeout(){
	    return waitTimeout;
	}        
        
        int getVersion(){
            if (strncmp(sdn_version(), "shmem", strlen("shmem")) == 0){
                return 0;
            }

            if (strncmp(sdn_version(), "socket", strlen("socket")) == 0){
                return 1;
            }

            if (strncmp(sdn_version(), "dolphin", strlen("dolphin")) == 0){
                return 2;
            }
            return -1;
        }
    protected:
	/**
	 * This function will configure the block based on SDN library used.
	 * 
	 * Receive: pointer to block
	 *          segment size
	 * Return:  1 on success
	 *          0 on error
	**/
        int configure(sdn_block_t _block, unsigned int _segmentSize){
	    int err = 0;
	    if (!_block){
		printFatalError("StreamingDriverSDN::configure. Block is NULL.\n");
	        return 0;
	    }

	    // Configuration key/value pairs is implementation specific.
	    // Refer to implementation documentation for details.
	    char str[20];
        long auxSegSize = _segmentSize + headerSize;
	    sprintf(str, "%ld", auxSegSize);
//	    sprintf(str, "%ld", _segmentSize);        
	    err = sdn_configure(_block, "segment_size", str);
	    if (err != SDN_SUCCESS && err != 22){
		printFatalError("StreamingDriverSDN::configure: segment_size configuration error [%d] -> [%s].\n", err, sdn_strerror(err));
		return 0;
	    }else{
		printInformation("StreamingDriverSDN::configure: segment_size successefully configured with value [%s], header size = [%ld].\n", str, headerSize);
	    }

	    if (strncmp(sdn_version(), "socket", strlen("socket")) == 0)
	    {

		printInformation("StreamingDriverSDN::configure: Configuring socket: IP: %s , port: %d.\n", address, port);
		
		err = sdn_configure(_block, "multicast_ip", address);
	        if (err != SDN_SUCCESS && err != 22){
		    printFatalError("StreamingDriverSDN::configure: multicast_ip [%s] configuration error [%d] -> [%s].\n", address, err, sdn_strerror(err));	
		    return 0;
	    	}
		
		sprintf(str, "%d", port);
		err = sdn_configure(_block, "port", str);
		if (err != SDN_SUCCESS && err != 22){
		    printFatalError("StreamingDriverSDN::configure: port configuration error [%d] -> [%s].\n", err, sdn_strerror(err));
		    return 0;
	    	}
	    }
	    else if (strncmp(sdn_version(), "dolphin", strlen("dolphin")) == 0)
	    {
		printInformation("StreamingDriverSDN::configure: Configuring dolphin socket: nodes: [%s], remote node: [%d].\n", nodes, remoteNode);

		err = sdn_configure(_block, "nodes", nodes);
	        if (err != SDN_SUCCESS && err != 22){
		    printFatalError("StreamingDriverSDN::configure: Nodes configuration error [%d] -> [%s].\n", err, sdn_strerror(err));
		    return 0;
	    	}

	        sprintf(str, "%d", remoteNode);
		err = sdn_configure(_block, "remote_node", str);
	        if (err != SDN_SUCCESS && err != 22){
		    printFatalError("StreamingDriverSDN::configure: remote node configuration error [%d] -> [%s].\n", err, sdn_strerror(err));
		    return 0;
	    	}
	    }
	    else if (strncmp(sdn_version(), "shmem", strlen("shmem")) == 0)
	    {
		printInformation("StreamingDriverSDN::configure: Configuring shared memory: segment ID %s.\n", segmentID);

		err = sdn_configure(_block, "segment_id", segmentID);
	        if (err != SDN_SUCCESS && err != 22){
		    printFatalError("StreamingDriverSDN::configure: segment_id configuration error [%d] -> [%s].\n", err, sdn_strerror(err));
		    return 0;
	    	}
	    }
	    return 1;
	}

	/**
	 * This function will create the block based on SDN library used.
	 * 
	 * Receive: segment size
	 *          block type [receiver or publisher]
	 * Return:  connected SDN block or NULL on error.
	**/
        sdn_block_t createBlock(unsigned int _segmentSize, sdn_block_type_t _type){
	    sdn_block_t _block;
	    int err = 0;
	 
	    //create SDN block with type
	    err = sdn_create(&_block, _type);    
	    if (err != SDN_SUCCESS && err != 22){
		printFatalError("StreamingDriverSDN::createBlock: sdn_create error [%d] -> [%s].\n", err, sdn_strerror(err));
	        return NULL;
	    }else{
		printInformation("StreamingDriverSDN::createBlock: sdn_create success.\n");
	    }   

	    //configure SDN block with segment size
	    if (!configure(_block, _segmentSize))
	    {
		    printFatalError("StreamingDriverSDN::createBlock: block not configured.\n");

	        sdn_destroy(_block);
	        return NULL;
	    }else{
		    printInformation("StreamingDriverSDN::createBlock: block configured sucessefully.\n");
	    }    

	    //connect SDN block and get configured segment size
	    unsigned int seg_size;    
	    err = sdn_connect(_block, &seg_size, connectTimeout);
	    segmentSizeConf = seg_size;   
	    if (err != SDN_SUCCESS && err != 22)
	    {
		printFatalError("StreamingDriverSDN::sdn_createBlock: sdn_connect error [%d] -> [%s], segment size [%d].\n", err, sdn_strerror(err), segmentSizeConf);
	        sdn_destroy(_block);
	        return NULL;
	    }else{
		printInformation("StreamingDriverSDN::sdn_createBlock: sdn_connect sucecess with segment size [%d].\n", segmentSizeConf);
	    }

	    return _block;
	}


	/**
	 * This function will destroy SDN block
	 * 
	 * Receive: pointer to block
	**/
	//destroy data block
        void destroyBlock(sdn_block_t _block){
	    int err = 0;

	    //disconnect block before destroy
	    err = sdn_is_connected(_block);
	    if (err == SDN_SUCCESS && err != 22){
		printInformation("StreamingDriverSDN::destroyBlock: Disconnecting...\n");
	        sdn_disconnect(_block, 0);
	    }else{
		printFatalError("StreamingDriverSDN::destroyBlock: Already disconnected [%d] -> [%s].\n", err, sdn_strerror(err));
	    }
	    printInformation("StreamingDriverSDN::destroyBlock: Destroy...\n");

	    //destroy block
	    sdn_destroy(_block);
	}

	//print information messages
	//receive char array and args to use in vprintf or VAssertErrorCondition
	void printInformation(char * infoMsg, ...){
		va_list argList;
		va_start (argList, infoMsg);

		#ifdef _MARTE
		VAssertErrorCondition(Information, infoMsg, argList);
		#else
		vprintf(infoMsg, argList);
		#endif
		va_end(argList);
	} 

	//print error messages
	//receive char array and args to use in vprintf or VCStaticAssertErrorCondition
	void printFatalError(char * errMsg, ...){
		va_list argList;
		va_start (argList, errMsg);

		#ifdef _MARTE
		VCStaticAssertErrorCondition(FatalError, errMsg, argList);
		#else
		vprintf(errMsg, argList);
		#endif
		va_end(argList);
	} 
};


//Class to create SDN receiver
//Descend from SDNSocket
//Implements all receiver methods
class SDNSocketReceiver: public SDNSocket {
    
    public:
	/**
	 * This function will init receiver parameters and create receiver block
	 * 
	 * set block with created block pointer
	 *
	 * Return:  true  on success
	 *          false on error
	**/
        bool receiver_init(){

                //Modify
                if(block == NULL){
    	            printInformation("Receiver Init: [Version: %s] [Segment Size: %d].\n", sdn_version(), segmentSize);

            	    //create receiver block and set global pointer
    	            block = createBlock(segmentSize, SDN_BLOCK_TYPE_RECEIVER);
    	            if(block){
    		            printInformation("Receiver block created.\n");
    	                return true;
    	            }else{
    		            printFatalError("Receiver block creation error.\n");
    	                return false;
    	            }
                }else{
                       return true;
                }
	}
	/**
	 * This function will receive one block
	 *
	 * Return:  pointer to received date
	 *          
	**/
        void* receive(){    
	    //If block is not created return NULL
	    if (block)
	    {

	        volatile const void * data;
	        unsigned int integrity_id = 0;
	        int err;

	        // Set a timeout relative to current time
	        // If timeout = 0 -> blocking read
	        // If timeout > 0 -> waiting configurated timeout

    		//hpn_timestamp_t pubtime;
    		//hpn_timestamp_t rectime;

	        hpn_timestamp_t timeout;
	        timeout = 0;
	        timeout += waitTimeout;


	        // Wait for new data
	        err = sdn_wait_for_ready_read(block, &data, &integrity_id, timeout);

	        if (err != SDN_SUCCESS && err != 22)
	        {
			printFatalError("ERROR: can not read data. Message Received: OTHER [%d] -> [%s].\n", err, sdn_strerror(err));
			    return NULL;
	/*		if (err == ETIMEDOUT)
	            	{
		            //printf("Message Received: ETIMEDOUT [%d]\n", err);	    
	                    // In case of timeout, check whether publisher started to write
	                    // new message
	                    if (sdn_check_integrity(block, integrity_id) != SDN_SUCCESS)
	                    {
	                        //printf("Receiving new message...\n");
	                    }
	                }
	                else
	                {
		 	    printFatalError("ERROR: can not read data. Message Received: OTHER [%d].\n", err);
	                } */           
	        }

/*	        sdn_get_data_timestamps(block, &pubtime, &rectime);
	        printf("Packet traveling time: %ld - %ld = %ld nanoseconds.\n", rectime, pubtime, rectime - pubtime);
*/
	        // return pointer to data
	        return (void *) data;
	    }
	    
	    return NULL;
	}

	/**
	 * This function will wake_up from sdn_wait_for_ready_read if is blocked
	 *          
	**/
	void close(){

	    printInformation("SDNSocketReceiver closing.\n");

	    sdn_wakeup(block);        
	}

	/**
	 * This function will wake_up from sdn_wait_for_ready_read if is blocked
	 *          
	**/
	void destroy(){

	    printInformation("SDNSocketReceiver Destroying.\n");
	    destroyBlock(block); 
        block = NULL;
	}

	//Call base class constructor
        SDNSocketReceiver (){
	    SDNSocket();    
	}

	//destroy block and finalize sdn
        ~SDNSocketReceiver(){
	    destroyBlock(block); 
		sdn_finalize();
	}
};

//Class to create SDN publisher
//Descend from SDNSocket
//Implements all publisher methods
class SDNSocketPublisher: public SDNSocket {
        
    public:
	/**
	 * This function will init publisher parameters and create publisher block
	 * 
	 * set block with created block pointer
	 *
	 * Return:  true  on success
	 *          false on error
	**/
        bool publisher_init(){
	    printInformation("Publisher Init: [Version: %s] [Segment Size: %d].\n", sdn_version(), segmentSize);

	    //create publisher block and set global pointer
	    block = createBlock(segmentSize, SDN_BLOCK_TYPE_PUBLISHER);

	    if(block){
		printInformation("Publisher block created.\n");
	        return true;
	    }else{
		printFatalError("Publisher block creation error.\n");
	        return false;
	    }    
	}

	/**
	 * This function will publish buffer data in argument
	 *
	 * Receive: pointer to data to publish
	 * Return:  true  on success
	 *          false on error
	**/
        bool publish(void *buffer){
	    if (block)
	    {
		//get block address
	        volatile double * data = (volatile double *)sdn_get_write_address(block);
		//copy data to buffer
		memcpy((void *)data, buffer, segmentSizeConf);
		//call initiate write 
	        sdn_initiate_write(block);
		//publish block
	        sdn_publish(block);
		return true;
	    }else{
		printFatalError("Publisher block NULL.\n");
		return false;
	    }
	}

	//Call base class constructor
        SDNSocketPublisher (){
	    SDNSocket();    
	}

	//finalize sdn connection on publisher
        ~SDNSocketPublisher (){
		sdn_finalize(); 
	}    
};

#endif	/* SDNSOCKET_H */
