/*
 * Copyright 2007 The MathWorks, Inc.
 *
 * File: raccel.h
 *
 *  $Revision: 1.1.6.1 $
 *  $Date: 2011/01/21 17:56:00 $
 *
 * Abstract:
 *	Data structures used with the RSIM from file and from workspace block
 *      handling.
 *
 * Requires include files
 *	tmwtypes.h
 *	simstruc_type.h
 * Note including simstruc.h before rsim.h is sufficient because simstruc.h
 * includes both tmwtypes.h and simstruc_types.h.
 */

#ifndef __RAPID_UTILS_H__
#define __RAPID_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*=========*
 * Defines *
 *=========*/

#define RUN_FOREVER -1.0

#define QUOTE1(name) #name
#define QUOTE(name) QUOTE1(name)    /* need to expand name    */

#ifndef SAVEFILE
# define MATFILE2(file) #file ".mat"
# define MATFILE1(file) MATFILE2(file)
# define MATFILE MATFILE1(MODEL)
#else
# define MATFILE QUOTE(SAVEFILE)
#endif

#ifndef MAXSTRLEN
# define MAXSTRLEN 509
#endif

/*==========*
 * Typedefs *
 *==========*/

/* Blockath and filename pairs for To File, From File blocks */
typedef struct {
    char blockpath[MAXSTRLEN];
    char filename[MAXSTRLEN];
} FileInfo;

/* File Name Pairs for To File, From File blocks */
typedef struct {
    const char *inputString;
    char       *oldName;
    const char *newName;
    int_T      remapped; /* Used to verify that name was remapped */
} FNamePair;


/* From File Info (one per from file block) */
typedef struct {
    const char  *origFileName;
    const char  *newFileName;
    int         originalWidth;
    int         nptsTotal;
    int         nptsPerSignal;
    double      *tuDataMatrix;
} FrFInfo;


/* From Workspace Info (one per from workspace block) */
typedef struct {
    const char *origWorkspaceVarName;
    DTypeId    origDataTypeId;
    int        origIsComplex;
    int        origWidth;
    int        origElSize;
    int        nDataPoints;
    double     *time;    /* time vector must be double */
    void       *data;    /* data vector can be any type including complex */
    double     *valDims; /* valueDimensions vector is stored in double */
} FWksInfo;

typedef struct {
    void   *ur;                /* columns of inputs: real part        */
    void   *ui;                /* columns of inputs: imag part        */   
    double *time;              /* time vector of each row             */           
    int_T  nTimePoints;        
    int    uDataType;          
    int    complex; 
    int    currTimeIdx;       /* for interpolation */
} rtInportTUtable;

#define NUM_DATA_TYPES (9)

/* Information associated with parameters of a given data type */
typedef struct {
    /* data attributes */
    int  dataType;
    bool complex;
    int  dtTransIdx;
    int  elSize; /* for debugging */
    
    /* data */
    int  nEls;
    void *rVals;
    void *iVals;
} DTParamInfo;

/* Optionally one for the model */
typedef struct {
    int     errFlag;
    int     numParams;   /* total number of params                   */
    double  checksum[4]; /* model checksum                           */

    int         nTrans;    
    DTParamInfo *dtParamInfo;
} PrmStructData;


extern const char *rt_RapidReadFromFileBlockMatFile(const char *origFileName, 
                                          int originalWidth,
                                          FrFInfo *frFInfo);


extern const char *rt_RapidReadInportsMatFile(const char* inportFileName, int* matFileFormat);              

extern bool mxIsIntVector(const mxArray *pm);

extern void rt_Interpolate_Datatype(void   *x1, void   *x2, void   *yout,
    				    real_T t,   real_T t1,  real_T t2,
                                    int    outputDType);

extern int_T rt_getTimeIdx(real_T *timePtr, real_T t, int_T numTimePoints, 
                           int_T preTimeIdx, boolean_T interp);

extern void rt_RapidFreeGbls(int);

extern const char *rt_RapidCheckRemappings(void);

extern void rt_RapidReadMatFileAndUpdateParams(const SimStruct *S);
 
#ifdef __cplusplus
}
#endif

#endif /* __RAPID_UTILS_H__ */
