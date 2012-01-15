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
 * Serial port communication implementation
 */
#if !defined(SERIAL__)
#define SERIAL__

#include "Streamable.h"
#include "FString.h"


///#warning pack of structures set to 4

/** the parity choices */
enum SerialParityModes {
    /** */
    NoParity       = 0,

    /** */
    OddParity      = 1,

    /** */
    EvenParity     = 2,

    /** */
    MarkParity     = 3,

    /** */
    SpaceParity    = 4

};

/** the parity choices */
enum SerialStopBitsModes {
    /** 1 stop bit*/
    Stop1          = 0,

    /** 1.5 stop bits */
    Stop1_5        = 1,

    /** 2 sop bits */
    Stop2          = 2

};

/** the RTS modes */
enum SerialRTSModes {
    /** Disables the RTS line when the device is opened and leaves it disabled. */
    RTSDisabled    = 0,

    /** Enables the RTS line when the device is opened and leaves it on */
    RTSEnabled     = 1,

    /** Enables RTS handshaking. The driver raises the RTS line when the "type-ahead"
    (input) buffer is less than one-half full and lowers the RTS line when the buffer
    is more than three-quarters full. If handshaking is enabled, it is an error for the
    application to adjust the line by using the EscapeCommFunction function */
    RTSHandShake   = 2,

    /** Specifies that the RTS line will be high if bytes are available for transmission.
    After all buffered bytes have been sent, the RTS line will be low */
    RTSToggle      = 3
};

/** the RTS modes */
enum SerialDTRModes {
    /** Disables the DTR line when the device is opened and leaves it disabled. */
    DTRDisabled    = 0,

    /** Enables the DTR line when the device is opened and leaves it on */
    DTREnabled     = 1,

    /** Enables DTR handshaking. If handshaking is enabled, it is an error for
    the application to adjust the line by using the EscapeCommFunction function*/
    DTRHandShake   = 2

};

/** the DSR modes */
enum SerialDSRModes {
    /** Disables the DTR line when the device is opened and leaves it disabled. */
    DSRDisabled    = 0,

    /** If this bit is enabled, the communications driver is sensitive to the state
    of the DSR signal. The driver ignores any bytes received, unless the DSR modem
    input line is high */
    DSRSensitivity = 1,

    /** If this bit is set, the DSR (data-set-ready) signal is monitored for output
    flow control. If this member is TRUE and DSR is turned off, output is suspended
    until DSR is sent again.*/
    DSROutFlow     = 2,

    /** both DSROutFlow and DSRSensitivity */
    DSRBoth        = 3

};

/** the CTS modes */
enum SerialCTSModes {
    /**  */
    CTSDisabled    = 0,

    /** If this bit is 1, the CTS (clear-to-send) signal is monitored for output flow
        control. If this member is TRUE and CTS is turned off, output is suspended until
        CTS is sent again */
    CTSOutFlow     = 1

};

/** change bits */
enum SerialToggleMode {
    /**  */
    Unchanged      = 0,

    /** turn to 1 */
    Enable         = 1,

    /** turn to 0 */
    Disable        = 2
};

/** bits of the status*/
enum SerialStatuses {
    /** If this bit is set, transmission is waiting for the CTS (clear-to-send)
        signal to be sent. */
    CtsHold        = 0x01,

    /** If this bit is set, transmission is waiting for the DSR (data-set-ready)
        signal to be sent. */
    DsrHold        = 0x02,

    /** If this bit is set, transmission is waiting for the RLSD (receive-line-signal-detect)
        signal to be sent. (Tx waiting for DCD to be turned on)*/
    RlsdHold       = 0x04,

    /** If this bit is set, transmission is waiting because the XOFF character was received. */
    XoffHold       = 0x08,

    /** If this bit is set, transmission is waiting because the XOFF character was transmitted.
        (Transmission halts when the XOFF character is transmitted to a system that takes the
        next character as XON, regardless of the actual character.) */
    XoffSent       = 0x10,

    /** OS2 only Tx waiting because break being transmitted. See ASYNC_SETBREAKON */
    Break          = 0x20,

    /** OS2 only Character waiting to transmit immediately. See ASYNC_TRANSMITIMM */
    TxReady        = 0x40,

    /** OS2 only Receive waiting for DSR to be turned on */
    DsrReceive     = 0x80,

    /** WIN32 only, If this bit is set, the end-of-file (EOF) character has been received */
    Eof            = 0x100,

    /** If this bit is set, there is a character queued for transmission that has come to
        the communications device by way of the TransmitCommChar function. The
        communications device transmits such a character ahead of other characters in
        the device's output buffer */
    Txim           = 0x200,

};

class Serial;

extern "C" {
    /** */
    bool SerialCommitSettings(Serial &serial);

    /** 0 means untouched 1 means Turn ON 2 means Turn OFF */
    bool SerialToggleControlBits(Serial &serial,SerialToggleMode dtr, SerialToggleMode rts);

    /** */
    bool SerialQueryCommStatus(Serial &serial,SerialStatuses &status,uint32 &inputQueueSize,uint32 &outputQueueSize);

    /** buffer is read and copied into the selected stream. */
    bool SerialWrite(Serial &serial,const void* buffer,uint32 &size);

    /** buffer is written from the selected stream. */
    bool SerialRead(Serial &serial,void* buffer,uint32 &size);

    /** */
    bool SerialOpen(Serial &serial,const char *deviceName);

    /** close the device */
    bool SerialClose(Serial &serial);

    /** initialise from CDB */
    bool SerialObjectLoadSetup(Serial &serial,ConfigurationDataBase &info,StreamInterface *err);

}


/** a com stream driver */
OBJECT_DLL(Serial)
class Serial: public Streamable {
OBJECT_DLL_STUFF(Serial)

friend bool SerialCommitSettings(Serial &serial);
friend bool SerialToggleControlBits(Serial &serial,SerialToggleMode dtr, SerialToggleMode rts);
friend bool SerialQueryCommStatus(Serial &serial,SerialStatuses &status,uint32 &inputQueueSize,uint32 &outputQueueSize);
friend bool SerialWrite(Serial &serial,const void* buffer,uint32 &size);
friend bool SerialRead(Serial &serial,void* buffer,uint32 &size);
friend bool SerialOpen(Serial &serial,const char *deviceName);
friend bool SerialClose(Serial &serial);
friend bool SerialObjectLoadSetup(Serial &serial,ConfigurationDataBase &info,StreamInterface *err);


#if defined (_OS2)
    /** the handle of the serial */
    HFILE               port;
#elif defined (_WIN32)
    /** the handle of the serial */
    HANDLE              port;
#endif

    /** the device name */
    FString             deviceName;

    /** */
    int32               baudRate;

    /** */
    int32               dataBits;

    /** */
    int32               writeTimeoutMSec;

    /** */
    int32               readTimeoutMSec;

    /** */
    SerialParityModes   parity;

    /** */
    SerialStopBitsModes stopBits;

    /** */
    SerialDTRModes      dtrControlMode;

    /** */
    SerialDSRModes      dsrControlMode;

    /** */
    SerialRTSModes      rtsControlMode;

    /** */
    SerialCTSModes      ctsControlMode;

private:

    /** implements all the programmed changes */
    inline bool CommitSettings(){
        return SerialCommitSettings(*this);
    }

    /** 0 means untouched 1 means Turn ON 2 means Turn OFF */
    inline bool ToggleControlBits(SerialToggleMode dtr, SerialToggleMode rts){
        return SerialToggleControlBits(*this,dtr,rts);
    }

public:
    /** constructor */
    Serial(){
#if defined (_OS2)
        port                = (HFILE)0;
#elif defined (_WIN32)
        port                = INVALID_HANDLE_VALUE;
#endif
        dataBits            = 7;
        parity              = EvenParity;
        stopBits            = Stop1;
        baudRate            = 9600;
        writeTimeoutMSec    = 200;
        readTimeoutMSec     = 200;
//        duplex_mode         = DuplexDisabled;
        dtrControlMode      = DTRDisabled;
        dsrControlMode      = DSRDisabled;
        rtsControlMode      = RTSDisabled;
        ctsControlMode      = CTSDisabled;
    }

    /** destructor */
    ~Serial (){
        Close();
    }

    /** */
    bool Opened(){
#if defined (_OS2)
        return (port != (HFILE)0);
#elif defined (_WIN32)
        return (port != INVALID_HANDLE_VALUE);
#else
        return False;
#endif
    }

    /** the name of the device to open : COM1
        if NULL than it will open what was setup using ObjectLoadSetup */
    bool Open(const char *deviceName=NULL){
        return SerialOpen(*this,deviceName);
    }

    /** close the device */
    bool Close(){
        return SerialClose(*this);
    }

    /** Parameters are
        @param DeviceName e.g. COM1:
        @param BaudRate             int
        @param DataBits             5 6 7 8 9
        @param WriteTimeoutMSec     int
        @param ReadTimeoutMSec      int
        @param Parity               NoParity    OddParity   EvenParity   MarkParity     SpaceParity
        @param StopBits             Stop1       Stop1_5     Stop2
        @param RTSControlMode       RTSDisabled RTSEnabled  RTSToggle
        @param DTRControlMode       DTRDisabled DTREnabled  DTRHandShake
        @param DSRControlMode       DSRDisabled DSRSensitivity  DSROutFlow    DSRBoth
        @param CTSControlMode       CTSDisabled CTSOutFlow

    */
    virtual bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){
        return SerialObjectLoadSetup(*this,info,err);
    }

    /** Standard Object Save function. Uses a CDB to pass the initialisation parameters.
        The CDB information is read from the subtree that is currently addressed.
        For binary save/loads put the binaryy content in root.binary    */
    virtual bool ObjectSaveSetup(ConfigurationDataBase &info,StreamInterface *err){ return False; }

    /** stremable interface */

    /** True if the stream can be read from */
    virtual bool CanRead(){
        return True;
    }

    /** True if the stream can be written to */
    virtual bool CanWrite(){
        return True;
    }

    /** Size Seek SetSize not available */
    virtual bool CanSeek(){
        return False;
    }

    /** Reads a block of data: size is the maximum size. on return size is what was read
        timeout not supported yet */
    virtual bool        SSRead(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        return SerialRead(*this,buffer,size);
    }


    /** Writes a block of data: size is its size. on return size is what was written.
        timeout not supported yet */
    virtual bool        SSWrite(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        return SerialWrite(*this,buffer,size);
    }

    /** initialisation */

    /** bitRate */
    bool SetSpeed(int baudRate){
#if defined(_OS2) || defined(_WIN32)
        this->baudRate = baudRate;
        if (port != 0) return CommitSettings();
#endif
        return True;

    }

    /** */
    bool SetDataBits(int dataBits){
#if defined(_OS2) || defined(_WIN32)
        if (dataBits < 5) return False;
        if (dataBits > 8) return False;
        this->dataBits = dataBits;
        if (port != 0) return CommitSettings();
#endif
        return True;
    }

    /** */
    bool SetParityBits(SerialParityModes parity){
#if defined(_OS2) || defined(_WIN32)
        this->parity = parity;
        if (port != 0) return CommitSettings();
#endif
        return True;
    }

    /** */
    bool SetStopBits(SerialStopBitsModes stopBits){
#if defined(_OS2) || defined(_WIN32)
        this->stopBits = stopBits;
        if (port != 0) return CommitSettings();
#endif
        return True;

    }

    /** sets the serial control modes */
    bool SetControlModes(
        SerialDTRModes dtrControlMode,
        SerialRTSModes rtsControlMode,
        SerialDSRModes dsrControlMode,
        SerialCTSModes ctsControlMode){
#if defined(_OS2) || defined(_WIN32)
        this->dtrControlMode = dtrControlMode;
        this->rtsControlMode = rtsControlMode;
        this->dsrControlMode = dsrControlMode;
        this->ctsControlMode = ctsControlMode;
        if (port != 0) return CommitSettings();
#endif
        return True;
    }


    /** other */


    /** manual control of the lines  only if DTR/RTS control modes = disabled */
    inline bool EnableDTR(void){
        return ToggleControlBits(Enable,Unchanged);
    }

    /** */
    inline bool DisableDTR(void){
        return ToggleControlBits(Disable,Unchanged);
    }

    /** */
    inline bool EnableRTS(void){
        return ToggleControlBits(Unchanged,Enable);
    }

    /** */
    inline bool DisableRTS(void){
        return ToggleControlBits(Unchanged,Disable);
    }

    /** Retrieve status of commPort */
    inline bool QueryCommStatus(SerialStatuses &status,uint32 &inputQueueSize,uint32 &outputQueueSize){
        return SerialQueryCommStatus(*this,status,inputQueueSize,outputQueueSize);
    }

};

#endif

