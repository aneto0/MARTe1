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

#include "System.h"
#include "Processor.h"
#include "HRT.h"
#include "ErrorManagement.h"
#if defined(_MACOSX)
#include <sys/sysctl.h>
#endif

#if defined(INTEL_PLATFORM)

uint32 Processor_mSecTics;
uint64 Processor_HRTFrequency;
double Processor_HRTPeriod;

#if defined(_MSC_VER)

///
static inline void CPUID_(uint32 code,uint32 &A,uint32 &B,uint32 &C,uint32 &D){
   uint32 a,b,c,d;
    __asm {
         mov eax , code
         _emit 0x0F
         _emit 0xA2
         mov a,eax
         mov b,ebx
         mov c,ecx
         mov d,edx
    }
    A = a;
    B = b;
    C = c;
    D = d;
}
#elif (defined(_CY32) || defined(__EMX__) || defined(_RSXNT))
/// executes the CPUID function
static inline void CPUID_(uint32 type,uint32 &A,uint32 &B,uint32 &C,uint32 &D){
    asm(
        "mov %4,%%eax \n"
        "cpuid"
        : "=a"(A),"=b"(B),"=c"(C),"=d"(D)
        : "q"(type)
        : "eax","ebx","ecx","edx"
    );
}
#elif defined(_RTAI)
/// executes the CPUID function
static inline void CPUID_(uint32 type,uint32 &A,uint32 &B,uint32 &C,uint32 &D){
    int a[8];
    asm(
        "movl %%eax,16(%0)\n"
        "movl %%ebx,20(%0)\n"
        "movl %%ecx,24(%0)\n"
        "movl %%edx,28(%0)\n"
        "movl %1,%%eax\n"
        "cpuid\n"
        "movl %%eax, (%0)\n"
        "movl %%ebx, 4(%0)\n"
        "movl %%ecx, 8(%0)\n"
        "movl %%edx, 12(%0)\n"
        "movl 16(%0), %%eax\n"
        "movl 20(%0), %%ebx\n"
        "movl 24(%0), %%ecx\n"
        "movl 28(%0), %%edx\n"
        :
        : "r" (&a[0]) ,"r" (type)
        : "eax","ebx","ecx","edx"
        );
    A=a[0];
    B=a[1];
    C=a[2];
    D=a[3];
}
#else
static inline void CPUID_(uint32 code,uint32 &A,uint32 &B,uint32 &C,uint32 &D){
}
#endif


static class HRTCalibrator{
public:
    HRTCalibrator(){

#if defined(_WIN32)

        uint64 tt0,tt1,tt2,tt3,tt4,tt5,dTa,dTb;
        dTa = 0;
        dTb = 0;
        for (int i=0;i<50;i++){
            tt2 = HRTRead64();
            QueryPerformanceCounter((LARGE_INTEGER *)&tt0);
            tt3 = tt4=HRTRead64();
            while ((tt4-tt3)<100000) tt4 = HRTRead64(); // .5 ms at 200 Mhz
            QueryPerformanceCounter((LARGE_INTEGER *)&tt1);
            tt5 = HRTRead64();
            dTa += (tt1 -tt0);
            dTb += ((tt5+tt4) - (tt3+tt2))/2;
        }
        QueryPerformanceFrequency((LARGE_INTEGER *)&Processor_HRTFrequency);
        Processor_HRTFrequency *= dTb;
        Processor_HRTFrequency /= dTa;

        Processor_HRTFrequency += 999999;
        Processor_HRTFrequency /= 2000000;
        Processor_HRTFrequency *= 2000000;

        Processor_HRTPeriod = 1.0 / (int64)Processor_HRTFrequency;

#elif defined (_OS2)
        Timer t;
        t.Create();
        uint32 time  = t.SystemTime();
        while ((t.SystemTime() - time)==0);
        uint64 t1 = HRTRead64();
        while ((t.SystemTime() - time)<11);
        uint64 t2 = HRTRead64();
        t2 = t2 - t1;
        t2 *= 100;
        Processor_HRTFrequency = t2;
        Processor_HRTPeriod = 1.0 / Processor_HRTFrequency;
        t.Close();
#elif defined (_RTAI)
        //RT_SCHED_MODE
        Processor_HRTFrequency = nano2count(1000000000);
        Processor_HRTPeriod = 1.0 / Processor_HRTFrequency;
#elif defined(_LINUX)

#define LINUX_CPUINFO_BUFFER_SIZE 1023
        Processor_HRTFrequency = 0;
        Processor_HRTPeriod    = 0;

        char buffer[LINUX_CPUINFO_BUFFER_SIZE + 1];

        FILE *f;
        f=fopen("/proc/cpuinfo","r");
        uint32 size = LINUX_CPUINFO_BUFFER_SIZE;
        size = fread(buffer,size,1,f);
        fclose(f);

        const char *pattern = "MHz";
        char *p = strstr(buffer,pattern);
        if (p != NULL){
            p = strstr(p,":");
            p++;
            double f = atof(p);
            if(f != 0){
                f *= 1.0e6;
                Processor_HRTFrequency = (int64)f;
                Processor_HRTPeriod    = 1.0 / f;
            }
        }
#elif defined(_MACOSX) // This is just an example to use with my MacBook

	size_t size;
	int32 clockrate;
	size = sizeof clockrate;
	sysctlbyname("hw.cpufrequency", &clockrate, &size, NULL, 0);

	Processor_HRTFrequency = (int64)clockrate;
	Processor_HRTPeriod    = (double)(1.0/((double)Processor_HRTFrequency));

	//Processor_HRTFrequency = (int64)2000000000;
	//Processor_HRTPeriod    = (double)(1.0/((double)Processor_HRTFrequency));
#else
#error static class HRT constructor missing
#endif
//!CHECK!
//No natural 64 bits division in kernel space
#if defined(_RTAI)
        uint64 Processor_HRTFrequency_temp = Processor_HRTFrequency;
        do_div(Processor_HRTFrequency_temp, 1000);
        Processor_mSecTics = Processor_HRTFrequency_temp;
#else
        Processor_mSecTics = Processor_HRTFrequency / 1000;
#endif
    }
} hrtCalibrator;

static class CPUINFO{
public:
    uint32 maxRead;
    uint32 ident[3];
    uint32 terminator; // to 0 terminate the ident string...
    uint32 versionInfo;
    uint32 reserved[2];
    uint32 featureInfo;
    uint32 extra[6];

    void GetInfo(){
        terminator = 0;
        CPUID_(0,maxRead,ident[0],ident[2],ident[1]);
        if (maxRead > 0)
            CPUID_(1,versionInfo,reserved[0],reserved[1],featureInfo);
    }
    CPUINFO(){ GetInfo(); }

} cpuinf;

#endif // end if defined (INTEL_PLATFORM)


uint64 ProcessorClockRate(){
#if defined(INTEL_PLATFORM) || defined(_MACOSX)
    return Processor_HRTFrequency;
#elif defined(_VX5500) || defined(_V6X5500)
    return 1000000000;
#elif defined(_VX5100) || defined(_V6X5100)
    return 400000000;
#elif defined(_VX68K)
    return 16000000;
#else
#endif
}

/// the clock period in seconds
double ProcessorClockCycle(){
#if defined(INTEL_PLATFORM) || defined(_MACOSX)
    return Processor_HRTPeriod;
#elif defined(_VX5500) || defined(_V6X5500)
    return 1e-9;
#elif defined(_VX5100) || defined(_V6X5100)
    return 2.5e-9;
#elif defined(_VX68K)
    return 62.5e-9;
#else
#endif
}


/// the processor type
const char *ProcessorName(){
#if defined(INTEL_PLATFORM)
    if (cpuinf.maxRead == 2) return "PentiumPro";
    return "Pentium";
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)
    return "PowerPC_G4";
#elif defined(_VX68K)
    return "68040";
#elif defined(_SOLARIS)
    return "SPARC";
#else
#endif
}


/// the processor family INTEL/MOTOROLA/...
uint32 ProcessorFamily(){
#if defined(INTEL_PLATFORM)
    return FAMILY_INTEL_X86;
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)
    return FAMILY_MOTOROLA_PPC;
#elif defined(_VX68K)
    return FAMILY_MOTOROLA_68K;
#else
#endif
}


/// 1 = True - 0 = False - otherwise the value
intptr ProcessorCharacteristic(uint32 capId){
#if defined(INTEL_PLATFORM)
    switch(capId){
        case CPU_IDENT:         return (intptr)(&cpuinf.ident[0]);
        case CPU_FPU_ON_CHIP:   return ((cpuinf.featureInfo & 0x1)!=0);
        case CPU_ENH_V8086:     return ((cpuinf.featureInfo & 0x2)!=0);
        case CPU_DEBUG_EXT:     return ((cpuinf.featureInfo & 0x4)!=0);
        case CPU_PAGESZ_EXT:    return ((cpuinf.featureInfo & 0x8)!=0);
        case CPU_TSC_AVAIL:     return ((cpuinf.featureInfo & 0x10)!=0);
        case CPU_MSR_AVAIL:     return ((cpuinf.featureInfo & 0x20)!=0);
        case CPU_PAE_AVAIL:     return ((cpuinf.featureInfo & 0x40)!=0);
        case CPU_MCE_AVAIL:     return ((cpuinf.featureInfo & 0x80)!=0);
        case CPU_CX8_AVAIL:     return ((cpuinf.featureInfo & 0x100)!=0);
        case CPU_APIC_AVAIL:    return ((cpuinf.featureInfo & 0x200)!=0);
        case CPU_MTRR_AVAIL:    return ((cpuinf.featureInfo & 0x1000)!=0);
        case CPU_PGE_AVAIL:     return ((cpuinf.featureInfo & 0x2000)!=0);
        case CPU_MCAA_AVAIL:    return ((cpuinf.featureInfo & 0x4000)!=0);
        case CPU_CMOV_AVAIL:    return ((cpuinf.featureInfo & 0x8000)!=0);
        case CPU_MMX_AVAIL:     return ((cpuinf.featureInfo & 0x800000)!=0);
        case CPU_TYPE:          return ((cpuinf.versionInfo >> 12) & 0x3);
        case CPU_FAMILY_:       return ((cpuinf.versionInfo >>  8) & 0xF);
        case CPU_MODEL:         return ((cpuinf.versionInfo >>  4) & 0xF);
        case CPU_STEPPING:      return ((cpuinf.versionInfo >>  0) & 0xF);
        case CPU_FEATINFO:      return   cpuinf.featureInfo;
        case CPU_VERSINFO:      return   cpuinf.versionInfo;
        case CPU_INTEL:         return strcmp((char *)&cpuinf.ident[0],"GENUINEINTEL");
        case CPU_MAXREAD:       return cpuinf.maxRead;
    }
    return 0;
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)
    switch(capId){
        case CPU_IDENT:         return (intptr)"?";
        case CPU_FPU_ON_CHIP:   return 1;
        case CPU_INTEL:         return 0;
        default: return 0;
    }
    return 0;
#elif defined(_VX68K)
    return 0;
#else
    return 0;
#endif
}


/// use it on constructors used on static object: the order of initialization is not guaranteed
void ProcessorReScanCPU(){
#if defined(INTEL_PLATFORM)
    cpuinf.GetInfo();
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

#elif defined(_VX68K)

#else
#endif
}

/** The number of cpus avaible. Return -1 if unknown */
int32 ProcessorsAvailable()
{
#ifdef _RTAI
    return fcomm_get_number_of_online_cpus();
#elif defined(_LINUX) || defined(_MACOSX)
    return sysconf(_SC_NPROCESSORS_ONLN);
#elif defined (_WIN32)
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwNumberOfProcessors;
#endif
    return -1;
}


