/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE-EPICS that is included with this distribution. 
\*************************************************************************/
//
//  Example EPICS CA server
//
//
//  caServer
//  |
//  exServer
//
//  casPV
//  |
//  exPV-----------
//  |             |
//  exScalarPV    exVectorPV
//  |
//  exAsyncPV
//
//  casChannel
//  |
//  exChannel
//


//MARTE BAseLib
#include "System.h"
#include "BasicTypes.h"


//
// ANSI C
//
#include <string.h>
#include <stdio.h>

//
// EPICS
//
//#define epicsAssertAuthor "Jeff Hill johill@lanl.gov"
#include "gddAppFuncTable.h"
#include "smartGDDPointer.h"
#include "epicsTimer.h"
#include "casdef.h"
#include "epicsAssert.h"
#include "resourceLib.h"
#include "tsMinMax.h"

#ifndef NELEMENTS
#   define NELEMENTS(A) (sizeof(A)/sizeof(A[0]))
#endif

//
// info about all pv in this server
//
enum excasIoType { excasIoSync, excasIoAsync };

class exPV;
class exServer;

//
// pvInfo 
// 
class pvInfo {
public: 
        
    pvInfo ( double scanPeriodIn, const  char * pNameIn, const char * pUnitsIn, 
        aitFloat32  hoprIn, aitFloat32  loprIn,
        double hihiIn, double highIn, double lowIn, double loloIn,
        aitEnum typeIn, excasIoType ioTypeIn, unsigned  countIn );
    
    pvInfo ( double scanPeriodIn, const  char * pNameIn, const char * pUnitsIn, 
        aitFloat32  hoprIn, aitFloat32  loprIn,
        double hihiIn, double highIn, double lowIn, double loloIn,
        aitEnum hhsvIn, aitEnum llsvIn, aitEnum hsvIn, aitEnum lsvIn,
        /*double lalmIn,*/ double hystIn,
        /*double alstIn,*/ double adelIn, /*double mlstIn,*/ double mdelIn,
        aitEnum acksIn, aitEnum acktIn, int precIn,
        aitEnum typeIn, excasIoType ioTypeIn, unsigned countIn );
    
    pvInfo (  const pvInfo  & copyIn  );
    ~pvInfo ();
    double getScanPeriod () const; 
    const  char * getName () const;
    const  char * getUnits () const; 
    double  getHopr () const; 
    double getLopr () const; 
    double  getHihi () const; 
    double getHigh () const; 
    double  getLow () const; 
    double getLolo () const; 
    aitEnum getType () const; 
    
    excasIoType  getIOType () const; 
    unsigned getElementCount () const; 
    void unlinkPV (); 
    exPV *createPV  ( exServer &  exCAS, bool  preCreateFlag, 
        bool  scanOn, double  asyncDelay ); 
    void deletePV ();
    exPV *getPV () { return pPV; };
    
  //  char * buffer;
  //  BasicTypeDescriptor btd;
    
private:
    const double scanPeriod;
    aitEnum scan;
    
    // TODO devono essere allocati qui: pvInfo e responsabile della loro creazione e distruzione
    // poteva andare bene per reference nell'implementazione precedente se erano statici a livello di routine
    // ma ora non è più così..
     char * pName;
     char * pUnits;
     char * pDesc;
    
    const double hopr;
    const double lopr;
    
    const double hihi;
    const double high;
    const double low;
    const double lolo;

public:
    aitEnum hhsv;
    aitEnum llsv;
    aitEnum hsv;
    aitEnum lsv;
    
    double lalm; // last alarm value
    double hyst; // hysteresis value
    double alst; // last archived value
    double adel; // archive deadband
    double mlst; // last monitored value
    double mdel; // monitor deadband
    
    aitEnum acks;
    aitEnum ackt; // YESorNO
    
    int prec;
        
private:    
    aitEnum type;
    const excasIoType ioType;
    const unsigned elementCount;
    exPV * pPV;
    pvInfo & operator = ( const pvInfo & );
};

//
// pvEntry 
//
// o entry in the string hash table for the pvInfo
// o Since there may be aliases then we may end up
// with several of this class all referencing 
// the same pv info class (justification
// for this breaking out into a seperate class
// from pvInfo)
//
class pvEntry // X aCC 655
    : public stringId, public tsSLNode < pvEntry > {
public:
    pvEntry ( pvInfo  &infoIn, exServer & casIn, 
            const char * pAliasName );
    ~pvEntry();
    pvInfo & getInfo() const { return this->info; }
    void destroy ();

private:
    pvInfo & info;
    exServer & cas;
    pvEntry & operator = ( const pvEntry & );
    pvEntry ( const pvEntry & );
};


//
// exPV
//
class exPV : public casPV, public epicsTimerNotify, 
    public tsSLNode < exPV > {
public:
    exPV ( exServer & cas, pvInfo & setup, 
        bool preCreateFlag, bool scanOn );
    virtual ~exPV();

    void show ( unsigned level ) const;

    //
    // Called by the server libary each time that it wishes to
    // subscribe for PV the server tool via postEvent() below.
    //
    caStatus interestRegister ();

    //
    // called by the server library each time that it wishes to
    // remove its subscription for PV value change events
    // from the server tool via caServerPostEvents()
    //
    void interestDelete ();

    aitEnum bestExternalType () const;

    //
    // chCreate() is called each time that a PV is attached to
    // by a client. The server tool must create a casChannel object
    // (or a derived class) each time that this routine is called
    //
    // If the operation must complete asynchronously then return
    // the status code S_casApp_asyncCompletion and then
    // create the casChannel object at some time in the future
    //
    //casChannel *createChannel ();

    //
    // This gets called when the pv gets a new value
    //
    caStatus update ( const gdd & , bool processEvent, bool updateValue);

    //
    // Gets called when we add noise to the current value
    //
    virtual void scan () = 0;
    
    //
    // If no one is watching scan the PV with 10.0
    // times the specified period
    //
    double getScanPeriod ();

    caStatus read ( const casCtx &, gdd & protoIn );

    caStatus readNoCtx ( smartGDDPointer pProtoIn );

    caStatus write ( const casCtx &, const gdd & value );

    void destroy ();

    const pvInfo & getPVInfo ();

    const char * getName() const;
    
    
    caStatus putAckt ( const gdd & valueIn ); 
    caStatus putAcks ( const gdd & valueIn );
    

    static void initFT();

    casChannel * createChannel ( const casCtx &ctx,
        const char * const pUserName, 
        const char * const pHostName );

protected:
    smartGDDPointer pValue;
    exServer & cas;
    epicsTimer & timer;
    pvInfo & info; 
    bool interest;
    bool preCreate;
    bool scanOn;
    static epicsTime currentTime;

    virtual caStatus updateValue ( const gdd & ) = 0;

private:

    //
    // scan timer expire
    //
    expireStatus expire ( const epicsTime & currentTime );

    //
    // Std PV Attribute fetch support
    //
    gddAppFuncTableStatus getPrecision(gdd &value);
    gddAppFuncTableStatus getHighLimit(gdd &value);
    gddAppFuncTableStatus getLowLimit(gdd &value);
    gddAppFuncTableStatus getHighAlarm ( gdd & value );
    gddAppFuncTableStatus getLowAlarm ( gdd & value );
    gddAppFuncTableStatus getHighWarning ( gdd & value );
    gddAppFuncTableStatus getLowWarning ( gdd & value );
    gddAppFuncTableStatus getUnits(gdd &value);
    gddAppFuncTableStatus getValue(gdd &value);
    gddAppFuncTableStatus getEnums(gdd &value);
    gddAppFuncTableStatus getAckt (gdd & prec);
    gddAppFuncTableStatus getAcks (gdd & prec); 
  //  gddAppFuncTableStatus getTimeStamp (gdd & prec);
    
    
    
    exPV & operator = ( const exPV & );
    exPV ( const exPV & );

    //
    // static
    //
    static gddAppFuncTable<exPV> ft;
    static char hasBeenInitialized;
};

//
// exScalarPV
//
class exScalarPV : public exPV {
public:
    exScalarPV ( exServer & cas, pvInfo &setup, 
        bool preCreateFlag, bool scanOnIn ) :
        exPV ( cas, setup, 
            preCreateFlag, scanOnIn) {}
    void scan();
private:
    caStatus updateValue ( const gdd & );
    exScalarPV & operator = ( const exScalarPV & );
    exScalarPV ( const exScalarPV & );
};

//
// exVectorPV
//
class exVectorPV : public exPV {
public:
    exVectorPV ( exServer & cas, pvInfo &setup, 
        bool preCreateFlag, bool scanOnIn ) :
        exPV ( cas, setup, 
            preCreateFlag, scanOnIn) {}
    void scan();

    unsigned maxDimension() const;
    aitIndex maxBound (unsigned dimension) const;

private:
    caStatus updateValue ( const gdd & );
    exVectorPV & operator = ( const exVectorPV & );
    exVectorPV ( const exVectorPV & );
};

//
// exServer
//
class exServer : private caServer {
public:
    //we have removed the unneeded parameters
    /*exServer ( const char * const pvPrefix, 
        unsigned aliasCount, bool scanOn,
        bool asyncScan, double asyncDelay,
        unsigned maxSimultAsyncIO );
    */
    exServer (bool scanOn,
        bool asyncScan, double asyncDelay,
        unsigned maxSimultAsyncIO );
    ~exServer ();
    void show ( unsigned level ) const;
    void removeIO ();
    
    void removeAliasName ( pvEntry & entry );
    // todo change return type to integer o pvEntry.. dato che serve in removeAliasName..
    void installAliasName ( pvInfo & info, const char * pAliasName );

    class epicsTimer & createTimer ();
	void setDebugLevel ( unsigned level );

    void destroyAllPV ();
    
    unsigned maxSimultAsyncIO () const;

    
    pvExistReturn pvExistTest ( const casCtx &, 
        const caNetAddr &, const char * pPVName );
    pvExistReturn pvExistTest ( const casCtx &, 
        const char * pPVName );
//    pvExistReturn pvExistTest ( const char * pPVName ); // used by the prototype
    pvExistReturn pvExistTest (const char * pPVName );
    
    
private:
    resTable < pvEntry, stringId > stringResTbl;
    epicsTimerQueueActive * pTimerQueue;
    
    // TODO add eventQueue
    
    unsigned simultAsychIOCount;
    const unsigned _maxSimultAsyncIO;
    double asyncDelay;
    bool scanOn;
    
    pvAttachReturn pvAttach ( const casCtx &, 
        const char * pPVName );

    exServer & operator = ( const exServer & );
    exServer ( const exServer & );

/*   
    //
    // list of pre-created PVs
    //
    static pvInfo pvList[];
    static const unsigned pvListNElem;
    
    //
    // on-the-fly PVs 
    //
    static pvInfo bill;
    static pvInfo billy;
    static pvInfo bloater;
    static pvInfo bloaty;
    static pvInfo boot;
    static pvInfo booty;
*/
};

//
// exAsyncPV
//
class exAsyncPV : public exScalarPV {
public:
    exAsyncPV ( exServer & cas, pvInfo &setup, 
        bool preCreateFlag, bool scanOnIn, double asyncDelay );
    caStatus read ( const casCtx & ctxIn, gdd & protoIn );
    caStatus write ( const casCtx & ctxIn, const gdd & value );
    caStatus writeNotify ( const casCtx & ctxIn, const gdd & value );
    void removeReadIO();
    void removeWriteIO();
    caStatus updateFromAsyncWrite ( const gdd & );
private:
    double asyncDelay;
    smartConstGDDPointer pStandbyValue;
    unsigned simultAsychReadIOCount;
    unsigned simultAsychWriteIOCount;
    exAsyncPV & operator = ( const exAsyncPV & );
    exAsyncPV ( const exAsyncPV & );
};

//
// exChannel
//
class exChannel : public casChannel{
public:
    exChannel ( const casCtx & ctxIn );
    void setOwner ( const char * const pUserName, 
        const char * const pHostName );
    bool readAccess () const;
    bool writeAccess () const;
private:
    exChannel & operator = ( const exChannel & );
    exChannel ( const exChannel & );
};

//
// exAsyncWriteIO
//
class exAsyncWriteIO : public casAsyncWriteIO, public epicsTimerNotify {
public:
    exAsyncWriteIO ( exServer &, const casCtx & ctxIn, 
            exAsyncPV &, const gdd &, double asyncDelay );
    ~exAsyncWriteIO ();
private:
    exAsyncPV & pv;
    epicsTimer & timer;
    smartConstGDDPointer pValue;
    expireStatus expire ( const epicsTime & currentTime );
    exAsyncWriteIO & operator = ( const exAsyncWriteIO & );
    exAsyncWriteIO ( const exAsyncWriteIO & );
};

//
// exAsyncReadIO
//
class exAsyncReadIO : public casAsyncReadIO, public epicsTimerNotify {
public:
    exAsyncReadIO ( exServer &, const casCtx &, 
            exAsyncPV &, gdd &, double asyncDelay );
    virtual ~exAsyncReadIO ();
private:
    exAsyncPV & pv;
    epicsTimer & timer;
    smartGDDPointer pProto;
    expireStatus expire ( const epicsTime & currentTime );
    exAsyncReadIO & operator = ( const exAsyncReadIO & );
    exAsyncReadIO ( const exAsyncReadIO & );
};

//
// exAsyncExistIO
// (PV exist async IO)
//
class exAsyncExistIO : public casAsyncPVExistIO, public epicsTimerNotify {
public:
    exAsyncExistIO ( const pvInfo & pviIn, const casCtx & ctxIn,
            exServer & casIn );
    virtual ~exAsyncExistIO ();
private:
    const pvInfo & pvi;
    epicsTimer & timer;
    exServer & cas;
    expireStatus expire ( const epicsTime & currentTime );
    exAsyncExistIO & operator = ( const exAsyncExistIO & );
    exAsyncExistIO ( const exAsyncExistIO & );
};

 
//
// exAsyncCreateIO
// (PV create async IO)
//
class exAsyncCreateIO : public casAsyncPVAttachIO, public epicsTimerNotify {
public:
    exAsyncCreateIO ( pvInfo & pviIn, exServer & casIn, 
        const casCtx & ctxIn, bool scanOnIn, double asyncDelay );
    virtual ~exAsyncCreateIO ();
private:
    pvInfo & pvi;
    epicsTimer & timer;
    exServer & cas;
    double asyncDelay;
    bool scanOn;
    expireStatus expire ( const epicsTime & currentTime );
    exAsyncCreateIO & operator = ( const exAsyncCreateIO & );
    exAsyncCreateIO ( const exAsyncCreateIO & );
};

inline pvInfo::pvInfo ( double scanPeriodIn, const char *pNameIn, const char *pUnitsIn, 
    aitFloat32 hoprIn, aitFloat32 loprIn,
    double hihiIn, double highIn, double lowIn, double loloIn,
    aitEnum typeIn, excasIoType ioTypeIn, 
    unsigned countIn )
    /*,
    aitEnum hhsvIn, aitEnum llsvIn, aitEnum hsvIn, aitEnum lsvIn,
    double hystIn, double adelIn, double mdelIn) */:

    scanPeriod ( scanPeriodIn ), /*pName ( pNameIn ), pUnits ( pUnitsIn),*/
    hopr ( hoprIn ), lopr ( loprIn ),
    hihi (hihiIn), high (highIn), low (lowIn), lolo (loloIn),
    type ( typeIn ), 
    ioType ( ioTypeIn ), elementCount ( countIn ), 
    pPV ( 0 )
{
	// NAME
	if ( pNameIn ) {
		pName = new char[ strlen(pNameIn) ];
		if ( pName )
			memcpy (pName, pNameIn, strlen(pNameIn) +1 );
	}
	else 
		pName = 0;
	// EGU
	if ( pUnitsIn ) {
		pUnits = new char[ strlen(pUnitsIn) ];
		if ( pUnits )
			memcpy (pUnits, pUnitsIn, strlen(pUnitsIn) +1 );
	}
	else 
		pUnits = 0;
	
	//buffer = 0;
	//btd = 0;
	prec = 4;
	
	//have a look at dbStatic/alarm.h 
	/*epicsSevNone = NO_ALARM,
	    epicsSevMinor,
	    epicsSevMajor,
	    epicsSevInvalid,
	    ALARM_NSEV
	    */
	
	acks = (aitEnum) 0; // menuAlarmSevr start without alarms NO CONFIG
	ackt = (aitEnum) 0; // menuYesNo initial == YES (see menuYesNo.h in /include)  YES = 1
	
	// load it with default values, but there are no default values... ?!?!
	//menuAlarmSevr
	hhsv = (aitEnum) 2; // tobe configured - alarm
	llsv = (aitEnum) 2; // tobe configured - alarm
	hsv = (aitEnum) 1; // tobe configured - alarm
	lsv = (aitEnum) 1; // tobe configured - alarm
	    
	//all double
	lalm = 0.0 ;
	hyst = 0.0001; // tobe configured - alarm
	alst = 0.00;
	adel = 0.000000001; // tobe configured - log
	mlst = 0.00;
	mdel = 0.001; // tobe configured - value
}

inline pvInfo::pvInfo ( double scanPeriodIn, const  char * pNameIn, const char * pUnitsIn, 
	aitFloat32  hoprIn, aitFloat32  loprIn,
	double hihiIn, double highIn, double lowIn, double loloIn,
	aitEnum hhsvIn, aitEnum llsvIn, aitEnum hsvIn, aitEnum lsvIn,
	/*double lalmIn,*/ double hystIn,
	/*double alstIn,*/ double adelIn, /*double mlstIn,*/ double mdelIn,
	aitEnum acksIn, aitEnum acktIn, int precIn,
	aitEnum typeIn, excasIoType ioTypeIn, unsigned countIn ) :

	scanPeriod ( scanPeriodIn ), /*pName ( pNameIn ), pUnits ( pUnitsIn),*/
	hopr ( hoprIn ), lopr ( loprIn ),
	hihi (hihiIn), high (highIn), low (lowIn), lolo (loloIn),
	hhsv (hhsvIn), llsv (llsvIn), hsv (hsvIn), lsv (lsvIn),
	lalm ( 0.0 ), hyst (hystIn),
	alst ( 0.0 ), adel (adelIn), mlst ( 0.0 ), mdel (mdelIn),
	acks (acksIn), ackt (acktIn), prec (precIn),
	type ( typeIn ), ioType ( ioTypeIn ), elementCount ( countIn ), 
	pPV ( 0 )
{
	// NAME
	if ( pNameIn ) {
		pName = new char[ strlen(pNameIn) ];
		if ( pName )
			memcpy (pName, pNameIn, strlen(pNameIn) +1 );
	}
	else 
		pName = 0;
	// EGU
	if ( pUnitsIn ) {
		pUnits = new char[ strlen(pUnitsIn) ];
		if ( pUnits )
			memcpy (pUnits, pUnitsIn, strlen(pUnitsIn) +1 );
	}
	else 
		pUnits = 0;	

	// stuff to be removed
	//buffer = 0;
//	btd = 0;
}


//
// for use when MSVC++ will not build a default copy constructor 
// for this class
//
inline pvInfo::pvInfo ( const pvInfo & copyIn ) :

    scanPeriod ( copyIn.scanPeriod ), /*pName ( copyIn.pName ), pUnits ( copyIn.pUnits ),*/
    hopr ( copyIn.hopr ), lopr ( copyIn.lopr ),
    hihi ( copyIn.hihi ), high ( copyIn.high ), 
    low ( copyIn.low ), lolo ( copyIn.lolo ), type ( copyIn.type ),
    ioType ( copyIn.ioType ), elementCount ( copyIn.elementCount ),
    pPV ( copyIn.pPV )
{
//	buffer = copyIn.buffer;
//	btd = copyIn.btd;
	prec = copyIn.prec;
	
	acks = copyIn.acks;
	ackt = copyIn.ackt;
	
	hhsv = copyIn.hhsv;
	llsv = copyIn.llsv;
	hsv = copyIn.hsv;
	lsv = copyIn.lsv;
	    
	//all double
	lalm = copyIn.lalm ;
	hyst = copyIn.hyst;
	alst = copyIn.alst;
	adel = copyIn.adel;
	mlst = copyIn.mlst;
	mdel = copyIn.mdel;
	
	if ( copyIn.pName ) {
		pName = new char[ strlen(copyIn.pName) ];
		if ( pName )
			memcpy( pName, copyIn.pName, strlen(copyIn.pName) +1 );
	}
	
	if ( copyIn.pUnits ) {
		pUnits = new char[ strlen(copyIn.pUnits) ];
		if ( pUnits )
			memcpy( pUnits, copyIn.pUnits, strlen(copyIn.pUnits) +1 );
	}

}

inline pvInfo::~pvInfo ()
{
    //
    // GDD cleanup gets rid of GDD's that are in use 
    // by the PV before the file scope destructer for 
    // this class runs here so this does not seem to 
    // be a good idea
    //
    //if ( this->pPV != NULL ) {
    //   delete this->pPV;
    //}
	
	// NAME
	if ( pName )
		delete [] pName;
	// EGU
	if ( pUnits ) 
		delete [] pUnits;
}

inline void pvInfo::deletePV ()
{
    if ( this->pPV != NULL ) {
        delete this->pPV;
    }
}

inline double pvInfo::getScanPeriod () const 
{ 
    return this->scanPeriod; 
}

inline const char *pvInfo::getName () const 
{ 
    return this->pName; 
}

inline const char *pvInfo::getUnits () const 
{ 
    return this->pUnits; 
} 

inline double pvInfo::getHopr () const 
{ 
    return this->hopr; 
}

inline double pvInfo::getLopr () const 
{ 
    return this->lopr; 
}

inline double pvInfo::getHihi () const 
{ 
    return this->hihi; 
}

inline double pvInfo::getHigh () const 
{ 
    return this->high; 
}

inline double pvInfo::getLow () const 
{ 
    return this->low; 
}

inline double pvInfo::getLolo () const 
{ 
    return this->lolo; 
}

inline aitEnum pvInfo::getType () const 
{ 
    return this->type;
}

inline excasIoType pvInfo::getIOType () const 
{ 
    return this->ioType; 
}

inline unsigned pvInfo::getElementCount () const 
{ 
    return this->elementCount; 
}

inline void pvInfo::unlinkPV () 
{ 
    this->pPV = NULL; 
}
//-----------------------------------------------------------------------------


inline pvEntry::pvEntry ( pvInfo  & infoIn, exServer & casIn, 
        const char * pAliasName ) : 
    stringId ( pAliasName ), info ( infoIn ), cas ( casIn ) 
{
    assert ( this->stringId::resourceName() != NULL );
}

inline pvEntry::~pvEntry ()
{
    this->cas.removeAliasName ( *this );
}

inline void pvEntry::destroy ()
{
    delete this;
}

inline void exServer::removeAliasName ( pvEntry & entry )
{
    pvEntry * pE;
    pE = this->stringResTbl.remove ( entry );
    assert ( pE == &entry );
}

inline double exPV::getScanPeriod ()
{
    double curPeriod = this->info.getScanPeriod ();
    if ( ! this->interest ) {
        curPeriod *= 10.0L;
    }
    return curPeriod;
}

inline caStatus exPV::readNoCtx ( smartGDDPointer pProtoIn )
{
    return this->ft.read ( *this, *pProtoIn );
}

inline const pvInfo & exPV::getPVInfo ()
{
    return this->info;
}

inline const char * exPV::getName () const
{
    return this->info.getName();
}

inline void exServer::removeIO()
{
    if ( this->simultAsychIOCount > 0u ) {
        this->simultAsychIOCount--;
    }
    else {
        fprintf ( stderr, 
            "simultAsychIOCount underflow?\n" );
    }
}

inline unsigned exServer :: maxSimultAsyncIO () const
{
    return this->_maxSimultAsyncIO;
}

inline exChannel::exChannel ( const casCtx & ctxIn ) : 
    casChannel(ctxIn) 
{
}

