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
// Example EPICS CA server
//
#include "exServer.h"

#include "gddApps.h"
#include "dbMapper.h"

//
// static data for exPV
//
char exPV::hasBeenInitialized = 0;
gddAppFuncTable<exPV> exPV::ft;
epicsTime exPV::currentTime;

//
// special gddDestructor guarantees same form of new and delete
//
class exFixedStringDestructor: public gddDestructor {
    virtual void run (void *);
};

//
// exPV::exPV()
//
exPV::exPV ( exServer & casIn, pvInfo & setup, 
            bool preCreateFlag, bool scanOnIn ) : 
    cas ( casIn ),
    timer ( cas.createTimer() ),
    info ( setup ),
    interest ( false ),
    preCreate ( preCreateFlag ),
    scanOn ( scanOnIn )
{
    //
    // no dataless PV allowed
    //
    assert (this->info.getElementCount()>=1u);

    //
    // start a very slow background scan 
    // (we will speed this up to the normal rate when
    // someone is watching the PV)
    //
    if ( this->scanOn && this->info.getScanPeriod () > 0.0 ) {
        this->timer.start ( *this, this->getScanPeriod() );
    }
}

//
// exPV::~exPV()
//
exPV::~exPV() 
{
    this->timer.destroy ();
    this->info.unlinkPV();
}

//
// exPV::destroy()
//
// this is replaced by a noop since we are 
// pre-creating most of the PVs during init in this simple server
//
void exPV::destroy()
{
    if ( ! this->preCreate ) {
        delete this;
    }
}


/*
 * Events that can be posted are:
 * #define DBE_VALUE    (1<<0)
 * #define DBE_ARCHIVE  (1<<1)
 * #define DBE_LOG      DBE_ARCHIVE
 * #define DBE_ALARM    (1<<2)
 * #define DBE_PROPERTY (1<<3)
 * see /src/db/dbEvent.c
 */
//
// exPV::update()
//
caStatus exPV::update ( const gdd & valueIn, bool processEvent, bool updateValue )
{
#   if DEBUG
        printf("Setting %s too:\n", this->info.getName().string());
        valueIn.dump();
#   endif

    aitInt16 sta = 0, sev = 0;     
	if ( this->pValue.valid() ) // check the validity
        this->pValue->getStatSevr(sta, sev); // fetch previous values

if ( updateValue ) {
//----------------------------------------------------------------------------- updateValue	
    caStatus status = this->updateValue ( valueIn );    
    if ( status || ( ! this->pValue.valid() ) ) {
        return status;
    }
}
 

if ( !processEvent || !(this->pValue.valid()) )
    return S_casApp_success;
//----------------------------------------------------------------------------- processEvent

    caServer * pCAS = this->getCAS();
    if ( this->interest == true && pCAS != NULL ) {
		casEventMask monitor_mask;
		
//monitor from aiRecord.c
aiRecord_monitor:


		// alarms
		aitInt16 nsta, nsev;
		this->pValue->getStatSevr(nsta, nsev); // fetch previous values
		if ( (nsta != sta) || (nsev != sev) )
			monitor_mask |= pCAS->alarmEventMask();

if ( (this->pValue)->isScalar() ) {
   		double delta;
   		double value;
   		this->pValue->get(value);
   		
   		// monitoring
    	delta = this->info.mlst - value;
    	if (delta < 0.0) delta = - delta;
    	if ( delta > this->info.mdel) {
    		monitor_mask |= pCAS->valueEventMask();
    		this->info.mlst = value;
    	}
    	    
    	//archiving
    	delta = this->info.alst - value;
    	if (delta < 0.0) delta = - delta;
    	if ( delta > this->info.adel ) {
    		monitor_mask |= pCAS->logEventMask();
    		this->info.alst = value;
    	}
}
else 
		monitor_mask |= pCAS->valueEventMask();
    		
    	if ( monitor_mask.eventsSelected() )
        	this->postEvent ( monitor_mask, *this->pValue );        	
    }

    return S_casApp_success;
}

//
// exScanTimer::expire ()
//
epicsTimerNotify::expireStatus
exPV::expire ( const epicsTime & /*currentTime*/ ) // X aCC 361
{
    this->scan();
    if ( this->scanOn && this->getScanPeriod() > 0.0 ) {
        return expireStatus ( restart, this->getScanPeriod() );
    }
    else {
        return noRestart;
    }
}

//
// exPV::bestExternalType()
//
aitEnum exPV::bestExternalType () const
{
    return this->info.getType ();
}

//
// exPV::interestRegister()
//
caStatus exPV::interestRegister ()
{
    if ( ! this->getCAS() ) {
        return S_casApp_success;
    }

    this->interest = true;
    if ( this->scanOn && this->getScanPeriod() > 0.0 && 
            this->getScanPeriod() < this->timer.getExpireDelay() ) {
        this->timer.start ( *this, this->getScanPeriod() );
    }

    return S_casApp_success;
}

//
// exPV::interestDelete()
//
void exPV::interestDelete()
{
    this->interest = false;
}

//
// exPV::show()
//
void exPV::show ( unsigned level ) const
{
    if (level>1u) {
        if ( this->pValue.valid () ) {
            printf ( "exPV: cond=%d\n", this->pValue->getStat () );
            printf ( "exPV: sevr=%d\n", this->pValue->getSevr () );
            printf ( "exPV: value=%f\n", static_cast < double > ( * this->pValue ) );
        }
        printf ( "exPV: interest=%d\n", this->interest );
        this->timer.show ( level - 1u );
    }
}

//
// exPV::initFT()
//
void exPV::initFT ()
{
    if ( exPV::hasBeenInitialized ) {
            return;
    }

    //
    // time stamp, status, and severity are extracted from the
    // GDD associated with the "value" application type.
    //
    
    // questo è un limite visto dal punto di vista del RECORD 
    // perche dovremmo associare un sacco di apptype che non esistono...
    
    // perciò dovremmo abbandonare questa interfaccia per passare ai record
    
    exPV::ft.installReadFunc ("value", &exPV::getValue);
    exPV::ft.installReadFunc ("precision", &exPV::getPrecision);
    exPV::ft.installReadFunc ("units", &exPV::getUnits);
    exPV::ft.installReadFunc ("enums", &exPV::getEnums);
    
    exPV::ft.installReadFunc ("graphicHigh", &exPV::getHighLimit);
    exPV::ft.installReadFunc ("graphicLow", &exPV::getLowLimit);
    exPV::ft.installReadFunc ("controlHigh", &exPV::getHighLimit);
    exPV::ft.installReadFunc ("controlLow", &exPV::getLowLimit);
    exPV::ft.installReadFunc ("alarmHigh", &exPV::getHighAlarm);
    exPV::ft.installReadFunc ("alarmLow", &exPV::getLowAlarm);
    exPV::ft.installReadFunc ("alarmHighWarning", &exPV::getHighWarning);
    exPV::ft.installReadFunc ("alarmLowWarning", &exPV::getLowWarning);
    
    // possiamo registrare anche le altre funzioni che si trovano nel mapper..
    // gdd/gddAppTale.h, per il momento ci interessano "ackt" e "acks" per la simulazione degli allarmi
    exPV::ft.installReadFunc ("ackt", &exPV::getAckt);
    exPV::ft.installReadFunc ("acks", &exPV::getAcks);
//    exPV::ft.installReadFunc ("timeStamp", &exPV::getTimeStamp);

    exPV::hasBeenInitialized = 1;
}


caStatus exPV::getAckt (gdd & prec) {
	prec.put(info.ackt);
	return S_cas_success;
}
caStatus exPV::getAcks (gdd & prec) {
	
//	printf("exPV::getAcks\n");
	
	prec.put(info.acks);
	return S_cas_success;
}

/*caStatus exPV::getTimeStamp (gdd & prec) {
	prec.put(info.timestamp);
	return S_cas_success;
}
*/
//
// exPV::getPrecision()
//
caStatus exPV::getPrecision ( gdd & prec )
{
    prec.put(info.prec);
    return S_cas_success;
}

// exPV::getHighLimit()
caStatus exPV::getHighLimit ( gdd & value )
{
    value.put(info.getHopr());
    return S_cas_success;
}
// exPV::getLowLimit()
caStatus exPV::getLowLimit ( gdd & value )
{
    value.put(info.getLopr());
    return S_cas_success;
}

// high alarm -> HIHI field
caStatus exPV::getHighAlarm ( gdd & value )
{
    value.put(info.getHihi());
    return S_cas_success;
}
// low alarm -> LOLO field
caStatus exPV::getLowAlarm ( gdd & value )
{
    value.put(info.getLolo());
    return S_cas_success;
}

// high warning alarm -> HIGH field
caStatus exPV::getHighWarning ( gdd & value )
{
    value.put(info.getHigh());
    return S_cas_success;
}
// low warning alarm -> LOW field
caStatus exPV::getLowWarning ( gdd & value )
{
    value.put(info.getLow());
    return S_cas_success;
}

//
// exPV::getUnits()
//
caStatus exPV::getUnits( gdd & units )
{
//    aitString str("furlongs", aitStrRefConstImortal);
//    units.put(str);
	units.put(info.getUnits());
    return S_cas_success;
}

//
// exPV::getEnums()
//
// returns the eneumerated state strings
// for a discrete channel
//
// The PVs in this example are purely analog,
// and therefore this isnt appropriate in an
// analog context ...
//
caStatus exPV::getEnums ( gdd & enumsIn )
{
    if ( this->info.getType () == aitEnumEnum16 ) {
        static const unsigned nStr = 2;
        aitFixedString *str;
        exFixedStringDestructor *pDes;

        str = new aitFixedString[nStr];
        if (!str) {
            return S_casApp_noMemory;
        }

        pDes = new exFixedStringDestructor;
        if (!pDes) {
            delete [] str;
            return S_casApp_noMemory;
        }

        strncpy (str[0].fixed_string, "off", 
            sizeof(str[0].fixed_string));
        strncpy (str[1].fixed_string, "on", 
            sizeof(str[1].fixed_string));

        enumsIn.setDimension(1);
        enumsIn.setBound (0,0,nStr);
        enumsIn.putRef (str, pDes);

        return S_cas_success;
    }

    return S_cas_success;
}

//
// exPV::getValue()
//
caStatus exPV::getValue ( gdd & value )
{
    caStatus status;

    if ( this->pValue.valid () ) {
        gddStatus gdds;

        gdds = gddApplicationTypeTable::
            app_table.smartCopy ( &value, & (*this->pValue) );
        if (gdds) {
            status = S_cas_noConvert;   
        }
        else {
            status = S_cas_success;
        }
    }
    else {
        status = S_casApp_undefined;
    }
    return status;
}

//
// exPV::write()
// (synchronous default)
//

// CALLED by CA mentre update e chiamata anche dal thread automatico di EPICS
caStatus exPV::write ( const casCtx &, const gdd & valueIn )
{
    /* apptype checking is required : array type do not have
     * alarms (at least I have to check also this).
     *
     * exPV::write -> exPV::update -> exPV::updateValue (exScalarPV or exVectorPV) 
     * meglio qui..
     * ca_array_put can only *put* base application type and acks/ackt
     */
	//printf("exPV::write application type %d [ACKT = %d .app, ACKS = %d .app]\n",
/*	printf("exPV::write application type %d [ACKT = %d .app %hd, ACKS = %d .app %hd]\n",
			valueIn.applicationType(),
			DBR_PUT_ACKT, gddDbrToAit[DBR_PUT_ACKT].app,
			DBR_PUT_ACKS, gddDbrToAit[DBR_PUT_ACKS].app );
*/
//	printf("exPV::app %d \n", valueIn.applicationType() ); //do not work ! :-| :-( very sad
	
	if ( valueIn.applicationType() == gddDbrToAit[DBR_PUT_ACKT].app) {
		return this->putAckt( valueIn );
	}
	else if (valueIn.applicationType() == gddDbrToAit[DBR_PUT_ACKS].app) {
	//else if (valueIn.applicationType() == 20) {
		return this->putAcks( valueIn );
	}
	else 
		return this->update ( valueIn, true, true );
}
// FROM dbAccess.c 
//static long putAckt(DBADDR *paddr, const unsigned short *pbuffer, long nRequest, long no_elements, long offset)
caStatus exPV::putAckt ( const gdd & valueIn ) 
{
    //dbCommon *precord = paddr->precord;
	aitUint16 ack_value;
	valueIn.get(ack_value);

    //if (*pbuffer == precord->ackt) return 0;
	if (ack_value == this->info.ackt)
		return 0; // 0 is success
	
    this->info.ackt = (aitEnum)ack_value;
    
    //db_post_events(precord, &precord->ackt, DBE_VALUE | DBE_ALARM); // I do not know how to do when not in a record
    if (!this->info.ackt && (this->info.acks > (aitEnum) this->pValue->getSevr()) ) {
        this->info.acks = (aitEnum)this->pValue->getSevr();
        
        //db_post_events(precord, &precord->acks, DBE_VALUE | DBE_ALARM);
    }
    
    //do a post event to people monitoring alarm changes...
    //db_post_events(precord, NULL, DBE_ALARM);
    caServer * pCAS = this->getCAS();
	casEventMask select ( pCAS->alarmEventMask());
	this->postEvent ( select, *this->pValue );        		
    
    return 0;
}

// FROM dbAccess.c
//static long putAcks(DBADDR *paddr, const unsigned short *pbuffer, long nRequest, long no_elements, long offset)
caStatus exPV::putAcks ( const gdd & valueIn )
{
    //dbCommon *precord = paddr->precord;
	aitUint16 ack_value;
	valueIn.get(ack_value);

    //if (*pbuffer >= precord->acks) {
	if (ack_value >= this->info.acks ) {
        this->info.acks = (aitEnum) 0;
        //db_post_events(precord, NULL, DBE_ALARM);
    	caServer * pCAS = this->getCAS();
    	casEventMask select ( pCAS->alarmEventMask());
    	this->postEvent ( select, *this->pValue );        		

//        db_post_events(precord, &precord->acks, DBE_VALUE | DBE_ALARM); // wait to implement records
    }
    return 0; // return success
}





//
// exPV::read()
// (synchronous default)
//
caStatus exPV::read ( const casCtx &, gdd & protoIn )
{
    return this->ft.read ( *this, protoIn );
}

//
// exPV::createChannel()
//
// for access control - optional
//
casChannel *exPV::createChannel ( const casCtx &ctx,
        const char * const /* pUserName */, 
        const char * const /* pHostName */ )
{
    return new exChannel ( ctx );
}

//
// exFixedStringDestructor::run()
//
// special gddDestructor guarantees same form of new and delete
//
void exFixedStringDestructor::run ( void * pUntyped )
{
    aitFixedString *ps = (aitFixedString *) pUntyped;
    delete [] ps;
}

