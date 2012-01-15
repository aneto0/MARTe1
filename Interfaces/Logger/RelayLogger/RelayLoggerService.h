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
#if !defined(RELAY_LOGGER_SERVICE)
#define RELAY_LOGGER_SERVICE

#include "WindowsServiceApplication.h"
#include "ConfigurationDataBase.h"

/** */
class RelayLoggerService: public WindowsServiceApplication {

    /** */
    ConfigurationDataBase cdb;

private:
    /** replace this with the main of the application */
    virtual bool ServiceInit();

    /** replace this with the start service */
    virtual bool ServiceStart();

    /** replace this with the stop service */
    virtual bool ServiceStop();

public:

    /** */
    RelayLoggerService(const char *serviceName,const char *title):
            WindowsServiceApplication(serviceName,title){
    }

};





#endif

