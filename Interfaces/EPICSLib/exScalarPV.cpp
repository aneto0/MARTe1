/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE-EPICS that is included with this distribution. 
\*************************************************************************/

#include <math.h>
#include <limits.h>
#include <stdlib.h>

#include "alarm.h"
#include "exServer.h"
#include "gddApps.h"

#define myPI 3.14159265358979323846

//
// SUN C++ does not have RAND_MAX yet
//
#if !defined(RAND_MAX)
//
// Apparently SUN C++ is using the SYSV version of rand
//
#if 0
#define RAND_MAX INT_MAX
#else
#define RAND_MAX SHRT_MAX
#endif
#endif

//
// exScalarPV::scan
//
void exScalarPV::scan()
{
    caStatus        status;
    double          radians;
    smartGDDPointer pDD;
    float           newValue;
    float           limit;
    int             gddStatus;

    //
    // update current time (so we are not required to do
    // this every time that we write the PV which impacts
    // throughput under sunos4 because gettimeofday() is
    // slow)
    //
//    this->currentTime = epicsTime::getCurrent ();

    pDD = new gddScalar ( gddAppType_value, aitEnumFloat64 );
    if ( ! pDD.valid () ) {
        return;
    }

    //
    // smart pointer class manages reference count after this point
    //
    gddStatus = pDD->unreference ();
    assert ( ! gddStatus );

   // consider every data type
/*    if ( this->info.buffer ) { // code by Anto
        switch ( (this->info.btd).Type() ) {
        case BTDTInteger:
        	pDD->put( (int)*( (int32 *)this->info.buffer ) );
        	break;
        case BTDTFloat:
        default:
        	pDD->put( (float)*( (float *)this->info.buffer ) );
        	break;
        }
    	// switch in base a data type :-)
    	
    }
    else { // original code */
    	radians = ( rand () * 2.0 * myPI ) / RAND_MAX;
    	if ( this->pValue.valid () ) {
    		this->pValue->getConvert(newValue);
    	}
    	else {
    		newValue = 0.0f;
    	}
    	newValue += (float) ( sin (radians) / 10.0 );
    	limit = (float) this->info.getHopr ();
    	newValue = tsMin ( newValue, limit );
    	limit = (float) this->info.getLopr ();
    	newValue = tsMax ( newValue, limit );
        *pDD = newValue;
    //}

    aitTimeStamp gddts ( this->currentTime ); // TODO change it!!!!
    pDD->setTimeStamp ( & gddts );

// antonio code    
    if ( (this->info.getScanPeriod() > 0.0) ) {
    	status = this->update ( *pDD, true, false );
    	if (status!=S_casApp_success) {
    		errMessage ( status, "scalar scan update failed\n" );
    	}
    }
    
  // delete &((gddScalar ) *pDD);
   // delete &((gdd) *pDD);
}


 #define DBE_VALUE    (1<<0)
 #define DBE_ARCHIVE  (1<<1)
 #define DBE_LOG      DBE_ARCHIVE
 #define DBE_ALARM    (1<<2)
 #define DBE_PROPERTY (1<<3)

//
// exScalarPV::updateValue ()
//
// NOTES:
// 1) This should have a test which verifies that the 
// incoming value in all of its various data types can
// be translated into a real number?
// 2) We prefer to unreference the old PV value here and
// reference the incomming value because this will
// result in each value change events retaining an
// independent value on the event queue.
//
caStatus exScalarPV::updateValue ( const gdd & valueIn )
{
	
	// muoviamo il check per il data type???
	// mi sa di si... ?!? pero ce da fare lo stesso anche per gli scalars..
	
    //
    // Really no need to perform this check since the
    // server lib verifies that all requests are in range
    //
    if ( ! valueIn.isScalar() ) {
        return S_casApp_outOfBounds;
    }    
    
    if ( ! pValue.valid () ) {
        this->pValue = new gddScalar ( gddAppType_value, this->info.getType () );
        if ( ! pValue.valid () ) {
            return S_casApp_noMemory;
        }
    }

    this->pValue->put ( & valueIn );
    
    // TODO for a full implementation see aiRecord.c checkAlarms
    double value;
    this->pValue->get(value);
    
/* Antonio code   
    if ( value < this->info.getLolo() ) {
    	this->pValue->setStatSevr(LOLO_ALARM, MAJOR_ALARM);
    }
    else if ( value < this->info.getLow() ) {
    	this->pValue->setStatSevr(LOW_ALARM, MINOR_ALARM);
    }
    
    if ( value > this->info.getHihi() ) {
    	this->pValue->setStatSevr(HIHI_ALARM, MAJOR_ALARM);
    }
    else if ( value > this->info.getHigh() ) {
    	this->pValue->setStatSevr(HIGH_ALARM, MINOR_ALARM);
    }
 */ 
    
// original code from the aiRecord checkAlarms -----------------------------------------------------------
/* non considered for now the following lines   
    if (prec->udf) {
            recGblSetSevr(prec, UDF_ALARM, INVALID_ALARM);
            return;
    }
*/

#define recGblSetSevr(NSTA, NSEV) ((nsev<(NSEV)) ? (nsta=(NSTA),nsev=(NSEV),true) : false)    
    aitInt16 nsev, nsta;
    
    
aiRecord_checkAlarms:

    double hyst, lalm;
    double alev;
    epicsEnum16 asev;
    
    this->pValue->getStatSevr(nsta, nsev); // fetch previous values
    
	hyst = this->info.hyst; //hyst = prec->hyst;
	lalm = this->info.lalm; //lalm = prec->lalm;

	/* alarm condition hihi */
	asev = this->info.hhsv; //asev = prec->hhsv;
	alev = this->info.getHihi(); //alev = this->info.hihi; //prec->hihi;
	if (asev && (value >= alev || ((lalm == alev) && (value >= alev - hyst)))) {        	
		if ( recGblSetSevr(HIHI_ALARM, asev) )
			this->info.lalm = alev;
			goto recGbl_recGblResetAlarms;
	}

	/* alarm condition lolo */
	asev = this->info.llsv; //asev = prec->llsv;
	alev = this->info.getLolo(); //alev = this->info.lolo; //alev = prec->lolo;
	if (asev && (value <= alev || ((lalm == alev) && (value <= alev + hyst)))) {
		if ( recGblSetSevr(LOLO_ALARM, asev) )
			this->info.lalm = alev;
            goto recGbl_recGblResetAlarms;
	}

	/* alarm condition high */
	asev = this->info.hsv; //asev = prec->hsv;
	alev = this->info.getHigh(); // alev = this->info.high; // alev = prec->high;
	if (asev && (value >= alev || ((lalm == alev) && (value >= alev - hyst)))) {
		if ( recGblSetSevr(HIGH_ALARM, asev) )
			this->info.lalm = alev;
            goto recGbl_recGblResetAlarms;
	}

	/* alarm condition low */
	asev = this->info.lsv; //asev = prec->lsv;
	alev = this->info.getLow();//alev = this->info.low; // alev = prec->low;
	if (asev && (value <= alev || ((lalm == alev) && (value <= alev + hyst)))) {
		if ( recGblSetSevr(LOW_ALARM, asev) )
			this->info.lalm = alev;
            goto recGbl_recGblResetAlarms;
	}

	/* we get here only if val is out of alarm by at least hyst */
	this->info.lalm = value;

	
//recGblResetAlarms  from recGbl.c	
recGbl_recGblResetAlarms:

	epicsEnum16 stat_mask = 0;
	//epicsEnum16 val_mask = 0;

	if (this->pValue->getSevr() != nsev ) //if (prev_sevr != new_sev)
	{ 
		this->pValue->setSevr(nsev);
		stat_mask = DBE_ALARM;
		//db_post_events(pdbc, &pdbc->sevr, DBE_VALUE); // monitor the change of the PV .SEVR
	}
	if (this->pValue->getStat() != nsta )
	{
		this->pValue->setStat(nsta);
		stat_mask |= DBE_VALUE;
	}
	if (stat_mask)
	{
		//db_post_events(pdbc, &pdbc->stat, stat_mask); // monitor the change of the PV .STAT
		//val_mask = DBE_ALARM;
		
		// questo significa che se ackt è a zero  oppure nsev >= acks allora aggiorna acks		
		//previous code		if (!pdbc->ackt || new_sevr >= pdbc->acks) {
		if (!this->info.ackt || nsev >= this->info.acks) {
			this->info.acks = (aitEnum) nsev;
			//db_post_events(pdbc, &pdbc->acks, DBE_VALUE); //monitor the change of the PV .ACKS
		}
	}
	// e fino a qui mi cambia stat e sevr e acks -----------------------------------------------------------
    
    return S_casApp_success;
}

