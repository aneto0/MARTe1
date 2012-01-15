//******************************************************************************
//      $Log: MARTeService.h,v $
//      Revision 1.1  2009/04/21 14:08:52  aneto
//      MARTe startup as service support
//
//******************************************************************************

#if !defined(MARTE_SERVICE)
#define MARTE_SERVICE

#include "WindowsServiceApplication.h"
#include "ConfigurationDataBase.h"

/** */
class MARTeService: public WindowsServiceApplication {

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
    MARTeService(const char *serviceName,const char *title):
            WindowsServiceApplication(serviceName,title){
    }

};





#endif

