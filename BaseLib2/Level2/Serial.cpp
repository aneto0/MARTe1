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

#include "CDBExtended.h"
#include "ConfigurationDataBase.h"
#include "Serial.h"

OBJECTREGISTER(Serial,"$Id$")


#if defined (_OS2)
/** the name of the device to open : COM1 */
bool SerialOpen(Serial &serial,const char *deviceName){
    if (deviceName != NULL){
        serial.deviceName = strdup(deviceName);
    }

    ULONG dummy;
    APIRET rc = DosOpen((PSZ)serial.deviceName,&serial.port,(PULONG)&dummy,0,
          FILE_NORMAL,OPEN_ACTION_OPEN_IF_EXISTS,
          OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NOINHERIT |
          OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_READWRITE |
          OPEN_FLAGS_NO_CACHE      | OPEN_FLAGS_WRITE_THROUGH,NULL);
    if (rc !=0 )  CStaticAssertPlatformErrorCondition(FatalError,rc,"Serial::Open");
    if (rc == 0) return serial.CommitSettings();
    return (rc==0);
}

/** close the device */
bool SerialClose(Serial &serial){
    APIRET rc = DosClose(serial.port);
    if (rc != 0) CStaticAssertPlatformErrorCondition(FatalError,rc,"Serial::Close");
    return (rc==0);
}
#elif defined (_WIN32)

/** the name of the device to open : COM1 */
bool SerialOpen(Serial &serial,const char *deviceName){
    if (deviceName != NULL){
        serial.deviceName = strdup(deviceName);
    }

    serial.port = CreateFile(
      serial.deviceName.Buffer(),
      GENERIC_READ | GENERIC_WRITE,
      0,
      NULL,
      OPEN_ALWAYS,
      FILE_ATTRIBUTE_NORMAL,
      NULL
    );
    if (serial.port == INVALID_HANDLE_VALUE){
        CStaticAssertPlatformErrorCondition(FatalError,"Serial::Open(%s) failed",deviceName);
        return False;
    }
    return serial.CommitSettings();
}

/** close the device */
bool SerialClose(Serial &serial){
    CloseHandle(serial.port);
    return True;
}
#else
/** the name of the device to open : COM1 */
bool SerialOpen(Serial &serial,const char *deviceName){
    CStaticAssertErrorCondition(IllegalOperation,"Serial::Open not implemented");
    return False;
}

/** close the device */
bool SerialClose(Serial &serial){
    CStaticAssertErrorCondition(IllegalOperation,"Serial::Close not implemented");
    return False;
}
#endif

#if defined (_OS2)
/** buffer is read and copied into the selected stream. */
bool SerialWrite(Serial &serial,const void* buffer,uint32 &size){
    ULONG actual;
    APIRET rc;
    rc = DosWrite(serial.port,buffer,size,&actual);
    if (rc != 0) {
        CStaticAssertPlatformErrorCondition(FatalError,rc,"Serial::Write");
    }
    size = actual;
    return (rc==0);
};

/** buffer is written from the selected stream. */
bool SerialRead(Serial &serial,void* buffer,uint32 &size){
    ULONG actual;
    APIRET rc = DosRead(serial.port,buffer,size,&actual);
    if (rc != 0) {
        CStaticAssertPlatformErrorCondition(FatalError,rc,"Serial::Read");
    }
    size = actual;
    return (rc==0);
};
#elif defined (_WIN32)
/** buffer is read and copied into the selected stream. */
bool SerialWrite(Serial &serial,const void* buffer,uint32 &size){
    DWORD actual;
    BOOL flag = WriteFile(serial.port,buffer,size,&actual,NULL);
    if (flag == FALSE) {
        CStaticAssertPlatformErrorCondition(FatalError,"Serial::FileWrite");
    }
    size = actual;
    return (flag!=FALSE);
};

/** buffer is written from the selected stream. */
bool SerialRead(Serial &serial,void* buffer,uint32 &size){
    DWORD actual;
    BOOL flag = ReadFile(serial.port,buffer,size,&actual,NULL);
    if (flag==FALSE) {
        CStaticAssertPlatformErrorCondition(FatalError,"Serial::FileRead");
    }
    size = actual;
    // reading 0 means eof!
    if (size==0) return False;
    return (flag!=FALSE);
};

#else
/** buffer is read and copied into the selected stream. */
bool SerialWrite(Serial &serial,const void* buffer,uint32 &size){
    CStaticAssertErrorCondition(IllegalOperation,"Serial::SSWrite");
    return False;
};

/** buffer is written from the selected stream. */
bool SerialRead(Serial &serial,void* buffer,uint32 &size){
    CStaticAssertErrorCondition(IllegalOperation,"Serial::SSRead");
    return False;
};
#endif

#if defined(_OS2)

#pragma pack(1)

///
union AsyncCoreParams {
    struct  {
        uint32 speed;
        uint8  fraction;
    } BaudRate;
    struct  {
        uint8 data;
        uint8 parity;
        uint8 stop;
    } ControlBits;
    struct  {
        uint8 on_mask;
        uint8 off_mask;
    } ModemControl;
    struct  {
        uint16 write_timeout;
        uint16 read_timeout;
        uint8  flags1;
        uint8  flags2;
        uint8  flags3;
        uint8  error_replacement_character;
        uint8  break_replacement_character;
        uint8  xon_character;
        uint8  xoff_character;
    } DCBControl;
};

///
union AsyncCoreData {
    struct  {
    } BaudRate;
    struct  {
    } ControlBits;
    struct  {
        uint16 error;
    } ModemControl;
    struct  {
    } DCBControl;
    uint8 CommStatus;
    struct  {
        uint16 CharsInQueue;
        uint16 QueueSize;
    } queCount;
};

#pragma pack(4)




bool SerialCommitSettings(Serial &serial){
    AsyncCoreParams params;
    uint32 paramlength;
    uint32 datalength;

    params.ControlBits.data   = serial.dataBits;
    params.ControlBits.parity = serial.parity;
    params.ControlBits.stop   = serial.stopBits;

    APIRET rc = DosDevIOCtl(serial.port,IOCTL_ASYNC,ASYNC_SETLINECTRL,
    (PVOID)&params,(ULONG)sizeof(params),(PULONG)&paramlength,
    NULL  ,0  ,(PULONG)&datalength);
    if (rc != 0) {
        CStaticAssertPlatformErrorCondition(FatalError,"Serial::CommitSettings:CommitLineOptions");
        return False;
    }

    params.DCBControl.write_timeout = serial.writeTimeoutMsec / 10;
    params.DCBControl.read_timeout  = serial.readTimeoutMsec / 10;
    params.DCBControl.flags1 = serial.dtrControlMode    +
    ((int)serial.dsrControlMode & DSROutFlow)     * 0x10) +
    ((int)serial.dsrControlMode & DSRSensitivity) * 0x40) +
    ((int)serial.ctsControlMode & CTSOutFlow)     * 0x08);

    params.DCBControl.flags2 =
        /*XON_control_mode | */
        duplex_mode |
        (((int)serial.rtsControlMode) * 0x40);
    params.DCBControl.flags3 = 0x02; //0xF2;
    params.DCBControl.error_replacement_character = 0;
    params.DCBControl.break_replacement_character = 0;
    params.DCBControl.xon_character = 0;
    params.DCBControl.xoff_character = 0;
    APIRET rc = DosDevIOCtl(serial.port,IOCTL_ASYNC,ASYNC_SETDCBINFO,
    (PVOID)&params,(ULONG)sizeof(params),(PULONG)&paramlength,
    NULL,0,(PULONG)&datalength);
    if (rc != 0) {
        CStaticAssertPlatformErrorCondition(FatalError,"Serial::CommitSettings:CommitDCBOptions");
        return False;
    }

    params.BaudRate.speed    = serial.baudRate;
    params.BaudRate.fraction = 0;
    APIRET rc = DosDevIOCtl(serial.port,IOCTL_ASYNC,ASYNC_EXTSETBAUDRATE,
    (PVOID)&params,(ULONG)sizeof(params),(PULONG)&paramlength,
    NULL  ,0  ,(PULONG)&datalength);

    if (rc != 0) {
        CStaticAssertPlatformErrorCondition(FatalError,rc,"Serial::CommitSettings:SetSpeed");
        return False;
    }
    return True;
}

bool SerialToggleControlBits(Serial &serial,SerialToggleMode dtr, SerialToggleMode rts){
    int OnMask = 0;
    if (dtr == Enable) OnMask |= 1;
    if (rts == Enable) OnMask |= 2;
    int OffMask = 0xFF;
    if (dtr == Disable) OnMask -= 1;
    if (rts == Disable) OnMask -= 2;

    AsyncCoreParams params;
    AsyncCoreData data;
    uint32 paramlength;
    uint32 datalength;
    params.ModemControl.on_mask  = OnMask;
    params.ModemControl.off_mask = OffMask;
    APIRET rc = DosDevIOCtl(serial.port,IOCTL_ASYNC,ASYNC_SETMODEMCTRL,
    (PVOID)&params,(ULONG)sizeof(params),(PULONG)&paramlength,
    (PVOID)&data  ,(ULONG)sizeof(data)  ,(PULONG)&datalength);
    if (rc != 0) CStaticAssertPlatformErrorCondition(FatalError,rc,"Serial::SerialToggleControlBits");
    return (rc==0);
}

bool SerialQueryCommStatus(Serial &serial,SerialStatuses &status,uint32 &inputQueueSize,uint32 &outputQueueSize){
    AsyncCoreData data;
    uint32 paramlength;
    uint32 datalength;

    APIRET rc = DosDevIOCtl(serial.port,IOCTL_ASYNC,ASYNC_GETINQUECOUNT,
    (PVOID)NULL,(ULONG)0,(PULONG)&paramlength,
    (PVOID)&data  ,(ULONG)sizeof(data)  ,(PULONG)&datalength);

    if (rc != 0) {
        CStaticAssertPlatformErrorCondition(FatalError,rc,"Serial::QueryCommStatus:ASYNC_GETINQUECOUNT");
        return False;
    }
    inputQueueSize =  data.queCount.CharsInQueue;

    APIRET rc = DosDevIOCtl(serial.port,IOCTL_ASYNC,ASYNC_GETOUTQUECOUNT,
    (PVOID)NULL,(ULONG)0,(PULONG)&paramlength,
    (PVOID)&data  ,(ULONG)sizeof(data)  ,(PULONG)&datalength);

    if (rc != 0) {
        CStaticAssertPlatformErrorCondition(FatalError,rc,"Serial::QueryCommStatus:ASYNC_GETOUTQUECOUNT");
        return False;
    }
    outputQueueSize =  data.queCount.CharsInQueue;


    APIRET rc = DosDevIOCtl(serial.port,IOCTL_ASYNC,ASYNC_GETCOMMSTATUS,
    (PVOID)NULL,(ULONG)0,(PULONG)&paramlength,
    (PVOID)&data  ,(ULONG)sizeof(data)  ,(PULONG)&datalength);

    if (rc != 0) {
        CStaticAssertPlatformErrorCondition(FatalError,rc,"Serial::QueryCommStatus:ASYNC_GETCOMMSTATUS");
        return False;
    }


    status = (SerialStatuses)data.CommStatus;
    return True;
}

/*
bool Serial::StopTransmit(void){
    uint32 paramlength;
    uint32 datalength;
    APIRET rc = DosDevIOCtl(port,IOCTL_ASYNC,ASYNC_STOPTRANSMIT,
    (PVOID)NULL,(ULONG)0,(PULONG)&paramlength,
    (PVOID)NULL,(ULONG)0,(PULONG)&datalength);
    if (rc != 0) AssertDosErrorCondition(rc,"Serial::ToggleControlBits");
    return (rc==0);
};

bool Serial::StartTransmit(void){
    uint32 paramlength;
    uint32 datalength;
    APIRET rc = DosDevIOCtl(port,IOCTL_ASYNC,ASYNC_STARTTRANSMIT,
    (PVOID)NULL,(ULONG)0,(PULONG)&paramlength,
    (PVOID)NULL,(ULONG)0,(PULONG)&datalength);
    if (rc != 0) AssertDosErrorCondition(rc,"Serial::ToggleControlBits");
    return (rc==0);
};
*/


#elif defined(_WIN32)


bool SerialCommitSettings(Serial &serial){
    DCB dcb;

    if (serial.port == INVALID_HANDLE_VALUE ){
        CStaticAssertErrorCondition(FatalError,"Serial::CommitSettings:port == INVALID_HANDLE_VALUE ");
        return False;
    }

    if (GetCommState(serial.port,&dcb)==FALSE){
        CStaticAssertPlatformErrorCondition(FatalError,"Serial::CommitSettings:GetCommState failed ");
        return False;
    }
    dcb.DCBlength           = sizeof(DCB);
    dcb.BaudRate            = serial.baudRate;
    dcb.fBinary             = TRUE;
    dcb.fParity             = FALSE;
    dcb.fOutxCtsFlow        = serial.ctsControlMode;
    dcb.fOutxDsrFlow        = (((int)serial.dsrControlMode & DSROutFlow)!= 0);
    dcb.fDtrControl         = serial.dtrControlMode;
    dcb.fDsrSensitivity     = (((int)serial.dsrControlMode & DSRSensitivity)!= 0);
    dcb.fTXContinueOnXoff   = FALSE;
    dcb.fOutX               = FALSE;
    dcb.fInX                = FALSE;
    dcb.fErrorChar          = FALSE;
    dcb.fNull               = FALSE;
    dcb.fRtsControl         = serial.rtsControlMode;
    dcb.fAbortOnError       = FALSE;
    dcb.ByteSize            = serial.dataBits;
    dcb.Parity              = serial.parity;
    dcb.StopBits            = serial.stopBits;

    if (SetCommState(serial.port,&dcb) == FALSE){
        CStaticAssertPlatformErrorCondition(FatalError,"Serial::CommitSettings:SetCommState failed ");
        return False;
    }

    COMMTIMEOUTS commTim;

    if (!GetCommTimeouts(serial.port,&commTim)){
        CStaticAssertPlatformErrorCondition(FatalError,"Serial::CommitSettings:GetCommTimeouts failed ");
        return False;
    }

    commTim.ReadIntervalTimeout         = 10;
    commTim.ReadTotalTimeoutMultiplier  = 0;
    commTim.ReadTotalTimeoutConstant    = serial.readTimeoutMSec;
    commTim.WriteTotalTimeoutMultiplier = 0;
    commTim.WriteTotalTimeoutConstant   = serial.writeTimeoutMSec;

    if (!SetCommTimeouts(serial.port,&commTim)){
        CStaticAssertPlatformErrorCondition(FatalError,"Serial::CommitSettings:SetCommTimeouts failed ");
        return False;
    }

    return True;
}

bool SerialToggleControlBits(Serial &serial,SerialToggleMode dtr, SerialToggleMode rts){

    if (dtr == Enable){
        if (EscapeCommFunction(serial.port,5)!= TRUE){
            CStaticAssertPlatformErrorCondition(FatalError,"Serial::ToggleControlBits:EscapeCommFunction(port,5) failed ");
            return False;
        }
    }
    if (dtr == Disable){
        if (EscapeCommFunction(serial.port,6)!= TRUE){
            CStaticAssertPlatformErrorCondition(FatalError,"Serial::ToggleControlBits:EscapeCommFunction(port,6) failed ");
            return False;
        }
    }
    if (rts == Enable){
        if (EscapeCommFunction(serial.port,3)!= TRUE){
            CStaticAssertPlatformErrorCondition(FatalError,"Serial::ToggleControlBits:EscapeCommFunction(port,3) failed ");
            return False;
        }
    }
    if (rts == Disable){
        if (EscapeCommFunction(serial.port,4)!= TRUE){
            CStaticAssertPlatformErrorCondition(FatalError,"Serial::ToggleControlBits:EscapeCommFunction(port,4) failed ");
            return False;
        }
    }


    return True;
}


bool SerialQueryCommStatus(Serial &serial,SerialStatuses &status,uint32 &inputQueueSize,uint32 &outputQueueSize){

    COMSTAT comStat;

    DWORD error;
    if (ClearCommError(serial.port,&error,&comStat)!= TRUE){
        CStaticAssertPlatformErrorCondition(FatalError,"Serial::QueryCommStatus:ClearCommError(port,&error,&comStat) failed ");
        return False;
    }

    int statusCopy = 0;
    if (comStat.fCtsHold)  statusCopy |= CtsHold;
    if (comStat.fDsrHold)  statusCopy |= DsrHold;
    if (comStat.fRlsdHold) statusCopy |= RlsdHold;
    if (comStat.fXoffHold) statusCopy |= XoffHold;
    if (comStat.fXoffSent) statusCopy |= XoffSent;
    if (comStat.fEof)      statusCopy |= Eof;
    if (comStat.fTxim)     statusCopy |= Txim;

    status = (SerialStatuses)statusCopy;

    inputQueueSize = comStat.cbInQue;
    outputQueueSize = comStat.cbOutQue;

    return True;
};

#elif (_RTAI)
bool SerialQueryCommStatus(Serial &serial,SerialStatuses &status,uint32 &inputQueueSize,uint32 &outputQueueSize){
    return False;
}

bool SerialToggleControlBits(Serial &serial,SerialToggleMode dtr, SerialToggleMode rts){
    return False;
}

bool SerialCommitSettings(Serial &serial){
    return False;
}

#endif

static const int32 SerialParityModesList[]={
    NoParity   ,
    OddParity  ,
    EvenParity ,
    MarkParity ,
    SpaceParity,
    0
};

static const int32 SerialStopBitsModesList[]={
    Stop1  ,
    Stop1_5,
    Stop2  ,
    0
};


static const int32 SerialRTSModesList[]={
    RTSDisabled,
    RTSEnabled ,
    RTSToggle  ,
    0
};


static const int32 SerialDTRModesList[]={
    DTRDisabled ,
    DTREnabled  ,
    DTRHandShake,
    0
};


static const int32 SerialDSRModesList[]={
    DSRDisabled   ,
    DSRSensitivity,
    DSROutFlow    ,
    DSRBoth       ,
    0
};

static const int32 SerialCTSModesList[]={
    CTSDisabled,
    CTSOutFlow ,
    0
};

#if defined (_OS2) || defined (_WIN32)

static const char *SerialParityModesNameList[]={
    "NoParity"   ,
    "OddParity"  ,
    "EvenParity" ,
    "MarkParity" ,
    "SpaceParity",
    NULL
};

static const char *SerialStopBitsModesNameList[]={
    "Stop1"  ,
    "Stop1_5",
    "Stop2"  ,
    NULL
};

static const char * SerialRTSModesNameList[]={
    "RTSDisabled",
    "RTSEnabled" ,
    "RTSToggle"  ,
    NULL
};

static const char * SerialDTRModesNameList[]={
    "DTRDisabled" ,
    "DTREnabled"  ,
    "DTRHandShake",
    NULL
};

static const char * SerialDSRModesNameList[]={
    "DSRDisabled"   ,
    "DSRSensitivity",
    "DSROutFlow"    ,
    "DSRBoth"       ,
    NULL
};

static const char * SerialCTSModesNameList[]={
    "CTSDisabled",
    "CTSOutFlow" ,
    NULL
};

#endif

bool SerialObjectLoadSetup(Serial &serial,ConfigurationDataBase &info,StreamInterface *err){
#if defined (_OS2) || defined (_WIN32)
    CDBExtended &info2 = (CDBExtended &)info;
//    ConfigurationDataBase &info = *info_;

    FString deviceName;
    if (info->ReadArray(&deviceName,CDBTYPE_FString,NULL,0,"DeviceName")){
        if (serial.Opened()){
            CStaticAssertErrorCondition(InitialisationError,"Serial::ObjectLoadSetup:cannot change device name once it is already opened",serial.dataBits);
        } else {
            if (!SerialOpen(serial,deviceName.Buffer())) return False;
        }
    } else {
        if (!serial.Opened()){
            CStaticAssertErrorCondition(Warning,"Serial::ObjectLoadSetup:DeviceName unassigned");
        }
    }

    if (!info->ReadArray(&serial.baudRate,CDBTYPE_int32,NULL,0,"BaudRate")){
        CStaticAssertErrorCondition(Warning,"Serial::ObjectLoadSetup:BaudRate assigned to %i",serial.baudRate);
    }

    if (!info->ReadArray(&serial.dataBits,CDBTYPE_int32,NULL,0,"DataBits")){
        CStaticAssertErrorCondition(Warning,"Serial::ObjectLoadSetup:DataBits assigned to %i",serial.dataBits);
    }

    if (!info->ReadArray(&serial.writeTimeoutMSec,CDBTYPE_int32,NULL,0,"WriteTimeoutMSec")){
        CStaticAssertErrorCondition(Warning,"Serial::ObjectLoadSetup:WriteTimeoutMSec assigned to %i",serial.writeTimeoutMSec);
    }

    if (!info->ReadArray(&serial.readTimeoutMSec,CDBTYPE_int32,NULL,0,"ReadTimeoutMSec")){
        CStaticAssertErrorCondition(Warning,"Serial::ObjectLoadSetup:WriteTimeoutMSec assigned to %i",serial.readTimeoutMSec);
    }

    if (!info2.ReadOptions((int32 &)serial.parity,"Parity",SerialParityModesNameList,&SerialParityModesList[0],NoParity)){
        CStaticAssertErrorCondition(Warning,"Serial::ObjectLoadSetup:Parity assigned to %i",serial.parity);
    }

    if (!info2.ReadOptions((int32 &)serial.stopBits,"StopBits",SerialStopBitsModesNameList,&SerialStopBitsModesList[0],Stop1)){
        CStaticAssertErrorCondition(Warning,"Serial::ObjectLoadSetup:StopBits assigned to %i",serial.stopBits);
    }

    if (!info2.ReadOptions((int32 &)serial.rtsControlMode,"RTSControlMode",SerialRTSModesNameList,&SerialRTSModesList[0],RTSDisabled)){
        CStaticAssertErrorCondition(Warning,"Serial::ObjectLoadSetup:RTSControlMode assigned to %i",serial.rtsControlMode);
    }

    if (!info2.ReadOptions((int32 &)serial.dtrControlMode,"DTRControlMode",SerialDTRModesNameList,&SerialDTRModesList[0],DTRDisabled)){
        CStaticAssertErrorCondition(Warning,"Serial::ObjectLoadSetup:DTRControlMode assigned to %i",serial.dtrControlMode);
    }

    if (!info2.ReadOptions((int32 &)serial.dsrControlMode,"DSRControlMode",SerialDSRModesNameList,&SerialDSRModesList[0],DSRDisabled)){
        CStaticAssertErrorCondition(Warning,"Serial::ObjectLoadSetup:DSRControlMode assigned to %i",serial.dsrControlMode);
    }

    if (!info2.ReadOptions((int32 &)serial.ctsControlMode,"CTSControlMode",SerialCTSModesNameList,&SerialCTSModesList[0],CTSDisabled)){
        CStaticAssertErrorCondition(Warning,"Serial::ObjectLoadSetup:CTSControlMode assigned to %i",serial.ctsControlMode);
    }


    return SerialCommitSettings(serial);
#else
return True;
#endif
}



//


/* removed
const uint8 XONEnabled     = 0x3;
const uint8 XONDisabled    = 0x0;
    int         XONControlMode;

*/


