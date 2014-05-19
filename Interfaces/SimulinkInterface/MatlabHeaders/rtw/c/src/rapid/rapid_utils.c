/******************************************************************
 *
 *  File: rapid_utils.c
 *
 *  $Revision: 1.1.6.3.4.1 $
 *
 *  Abstract:
 *      - provide utility files common to rapid accelerator and RSim
 *        target
 *
 * Copyright 2007-2010 The MathWorks, Inc.
 ******************************************************************/

/*
 * This file is still using the old 32-bit mxArray API
 */
#define MX_COMPAT_32

/* INCLUDES */
#include  <stdio.h>
#include  <stdlib.h>

#include  <string.h>
#include  <math.h>
#include  <float.h>
#include  <ctype.h>

/*
 * We want access to the real mx* routines in this file and not their RTW
 * variants in rt_matrx.h, the defines below prior to including simstruc.h
 * accomplish this.
 */
#include  "mat.h"
#define TYPEDEF_MX_ARRAY
#define rt_matrx_h
#include "simstruc.h"
#undef rt_matrx_h
#undef TYPEDEF_MX_ARRAY

#include "dt_info.h"
#include  "rapid_utils.h"

extern mxClassID rt_GetMxIdFromDTypeIdForRSim(BuiltInDTypeId dTypeID); 
extern mxClassID rt_GetMxIdFromDTypeId(BuiltInDTypeId dTypeID); 

/* external variables */
extern const char  *gblFromWorkspaceFilename;
extern const char  *gblParamFilename;
extern const char  *gblInportFileName;

extern const int_T gblNumToFiles;
extern FNamePair   *gblToFNamepair;

extern const int_T gblNumFrFiles;
extern FNamePair   *gblFrFNamepair;

extern int_T         gblParamCellIndex;


/* total signal width of root inports */
extern int_T gblNumModelInputs;
/* total number of root inports */
extern int_T gblNumRootInportBlks;        
/* inport dimensions */
extern int_T gblInportDims[];
/* inport signal is real or complex */
extern int_T gblInportComplex[];
/* inport interpolation flag */
extern int_T gblInportInterpoFlag[];
/* simulink data type idx of input port signal */
extern int_T gblInportDataTypeIdx[];
/* inport block smaple time */
extern int_T gblInportContinuous[];

rtInportTUtable *gblInportTUtables = NULL;

#define INVALID_DTYPE_ID   (-10)
#define SINGLEVAR_MATRIX   (0)
#define SINGLEVAR_STRUCT   (1)
#define MULTIPLEVAR_STRUCT (2) 

static PrmStructData gblPrmStruct;


# define Interpolate(v1,v2,f1,f2) \
	    (((v1)==(v2))?((real_T)(v1)):\
                          (((f1)*((real_T)(v1)))+((f2)*((real_T)(v2)))))
# define InterpRound(v) (((v) >= 0) ? floor(((real_T)(v)) + 0.5): \
				       ceil(((real_T)(v)) - 0.5))

/*==================*
 * NON-Visible routines *
 *==================*/

/*
  This is taken from rt_matrx with an added multiplication factor
 */
static real_T rapid_eps(real_T t)
{
    return(((sizeof(double)==sizeof(real_T)) ? DBL_EPSILON:FLT_EPSILON)*t*16);
}


/* Function: rt_FreeParamStructs ===========================================
 * Abstract:
 *      Free and NULL the fields of all 'PrmStructData' structures.
 */
void rt_FreeParamStructs(PrmStructData *paramStructure)
{
    if (paramStructure != NULL) {
        int         i;
        int         nTrans       = paramStructure->nTrans;
        DTParamInfo *dtParamInfo = paramStructure->dtParamInfo;

        if (dtParamInfo != NULL) {
            for (i=0; i<nTrans; i++) {
                /*
                 * Must free "stolen" parts of matrices with
                 * mxFree (they are allocated with mxCalloc).
                 */
                mxFree(dtParamInfo[i].rVals);
                mxFree(dtParamInfo[i].iVals);
            }
            free(dtParamInfo);
        }

        paramStructure->nTrans      = 0;
        paramStructure->dtParamInfo = NULL;
    }
} /* end rt_FreeParamStructs */

/* Function: rt_ReadParamStructMatFile=======================================
 * Abstract:
 *  Reads a matfile containing a new parameter structure.  It also reads the
 *  model checksum and compares this with the RTW generated code's checksum
 *  before inserting the new parameter structure.
 *
 * Returns:
 *	NULL    : success
 *	non-NULL: error string
 */
const char *rt_ReadParamStructMatFile(PrmStructData **prmStructOut,
                                         int           cellParamIndex)
{
    int           nTrans;
    int           i;
    MATFile       *pmat              = NULL;
    mxArray       *pa                = NULL;
    const mxArray *paParamStructs    = NULL;
    PrmStructData *paramStructure    = NULL;
    const char    *result            = NULL; /* assume success */

    paramStructure = &gblPrmStruct;

    /**************************************************************************
     * Open parameter MAT-file, read checksum, swap rtP data for type Double *
     **************************************************************************/

    if ((pmat=matOpen(gblParamFilename,"rb")) == NULL) {
        result = "could not find MAT-file containing new parameter data";
        goto EXIT_POINT;
    }

    /*
     * Read the param variable. The variable name must be passed in
     * from the generated code.
     */
    if ((pa=matGetNextVariable(pmat,NULL)) == NULL ) {
        result = "error reading new parameter data from MAT-file "
            "(matGetNextVariable)";
        goto EXIT_POINT;
    }

    /* Should be 1x1 structure */
    if (!mxIsStruct(pa) ||
        mxGetM(pa) != 1 || mxGetN(pa) != 1 ) {
        result = "parameter variables must be a 1x1 structure";
        goto EXIT_POINT;
    }

    /* look for modelChecksum field */
    {
        const double  *newChecksum;
        const mxArray *paModelChecksum;

        if ((paModelChecksum = mxGetField(pa, 0, "modelChecksum")) == NULL) {
            result = "parameter variable must contain a modelChecksum field";
            goto EXIT_POINT;
        }

        /* check modelChecksum field */
        if (!mxIsDouble(paModelChecksum) || mxIsComplex(paModelChecksum) ||
            mxGetNumberOfDimensions(paModelChecksum) > 2 ||
            mxGetM(paModelChecksum) < 1 || mxGetN(paModelChecksum) !=4 ) {
            result = "invalid modelChecksum in parameter MAT-file";
            goto EXIT_POINT;
        }

        newChecksum = mxGetPr(paModelChecksum);

        paramStructure->checksum[0] = newChecksum[0];
        paramStructure->checksum[1] = newChecksum[1];
        paramStructure->checksum[2] = newChecksum[2];
        paramStructure->checksum[3] = newChecksum[3];
    }

    /*
     * Get the "parameters" field from the structure.  It is an
     * array of structures.
     */
    if ((paParamStructs = mxGetField(pa, 0, "parameters")) == NULL) {
        goto EXIT_POINT;
    }

    /*
     * If the parameters field is a cell array then pick out the cell
     * array pointed to by the cellParamIndex
     */
    if ( mxIsCell(paParamStructs) ) {
        /* check that cellParamIndex is in range */
        int size = mxGetM(paParamStructs) * mxGetN(paParamStructs);
        if (cellParamIndex > 0 && cellParamIndex <= size){
            paParamStructs = mxGetCell(paParamStructs, cellParamIndex-1);
        }else{
            result = "Invalid index into parameter cell array";
            goto EXIT_POINT;
        }
        if (paParamStructs == NULL) {
            result = "Invalid parameter field in parameter structure";
            goto EXIT_POINT;
        }
    }

    nTrans = mxGetNumberOfElements(paParamStructs);
    if (nTrans == 0) goto EXIT_POINT;

    /*
     * Validate the array fields - only check the first element of the
     * array since all elements of a structure array have the same
     * fields.
     *
     * It is assumed that if the proper data fields exists, that the
     * data is correct.
     */
    {
        mxArray *dum;

        if ((dum = mxGetField(paParamStructs, 0, "dataTypeName")) == NULL) {
            result = "parameters struct must contain a dataTypeName field";
            goto EXIT_POINT;
        }

        if ((dum = mxGetField(paParamStructs, 0, "dataTypeId")) == NULL) {
            result = "parameters struct must contain a dataTypeId field";
            goto EXIT_POINT;
        }

        if ((dum = mxGetField(paParamStructs, 0, "complex")) == NULL) {
            result = "parameters struct must contain a complex field";
            goto EXIT_POINT;
        }

        if ((dum = mxGetField(paParamStructs, 0, "dtTransIdx")) == NULL) {
            result = "parameters struct must contain a dtTransIdx field";
            goto EXIT_POINT;
        }

    }

    /*
     * Allocate the DTParamInfo's.
     */
    paramStructure->dtParamInfo = (DTParamInfo *)
        calloc(nTrans,sizeof(DTParamInfo));
    if (paramStructure->dtParamInfo == NULL) {
        result = "Memory allocation error";
        goto EXIT_POINT;
    }

    paramStructure->nTrans = nTrans;

    /*
     * Get the new parameter data for each data type.
     */
    paramStructure->numParams = 0;
    for (i=0; i<nTrans; i++) {
        double      *pr;
        mxArray     *mat;
        DTParamInfo *dtprmInfo = &paramStructure->dtParamInfo[i];

        /*
         * Grab the datatype id.
         */
        mat = mxGetField(paParamStructs,i,"dataTypeId");
        pr  = mxGetPr(mat);

        dtprmInfo->dataType = (int)pr[0];

        /*
         * Grab the complexity.
         */
        mat = mxGetField(paParamStructs,i,"complex");
        pr  = mxGetPr(mat);

        dtprmInfo->complex = (bool)pr[0];

        /*
         * Grab the data type transition index.
         */
        mat = mxGetField(paParamStructs,i,"dtTransIdx");
        pr  = mxGetPr(mat);

        dtprmInfo->dtTransIdx = (int)pr[0];

        /*
         * Grab the data and any attributes.  We "steal" the data
         * from the mxArray.
         */
        mat = mxGetField(paParamStructs,i,"values");
        if (mat) {
            dtprmInfo->elSize = mxGetElementSize(mat);
            dtprmInfo->nEls   = mxGetNumberOfElements(mat);

            dtprmInfo->rVals  = mxGetData(mat);
            dtprmInfo->iVals  = mxGetImagData(mat);
            mxSetData(mat,NULL);
            mxSetImagData(mat,NULL);
        } else {
            dtprmInfo->nEls   = 0;
            dtprmInfo->elSize = 0;

            dtprmInfo->rVals  = NULL;
            dtprmInfo->iVals  = NULL;
        }

        /*
         * Increment total element count.
         */
        paramStructure->numParams += dtprmInfo->nEls;
    }

EXIT_POINT:
    mxDestroyArray(pa);

    if (pmat != NULL) {
        matClose(pmat); pmat = NULL;
    }

    if (result != NULL) {
        rt_FreeParamStructs(paramStructure);
        paramStructure = NULL;
    }
    *prmStructOut = paramStructure;
    return(result);
} /* end rt_ReadParamStructMatFile */



/* Function: rt_VerifyTimeMonotone ============================================
 * Abstract:
 * Verify the time vector of inport MAT-file is monotone increase
 * Input argument: 1. Time vector head pointer  
 * 2. number of time point 3. timeData pointer Offset  4 file name  
 *  
 * Returns:
 *	NULL    : success, is monotone
 *	non-NULL: error string
 */
const char* rt_VerifyTimeMonotone(const double* timeDataPtr, 
                                  size_t numOfTimePoints,
                                  int_T timeDataPtrOffset,
                                  const char* inportFileName )
{    
    size_t i;
    static char  errmsg[1024];
    errmsg[0] = '\0'; /* assume success */
        
    /* 1. Verify that numOfTimePoints must >=1 */
    if (numOfTimePoints < 1){
        (void)sprintf(errmsg,"Time vector in MAT-file "
                      " '%s' must have at least one element.\n ",
                      inportFileName);
    }
    
    /*
     * Verify that the time vector is monotonically increasing.
     */    
    for (i=1; i<numOfTimePoints; i++) {
        if (timeDataPtr[i*timeDataPtrOffset] 
            < timeDataPtr[(i-1)*timeDataPtrOffset]) {
            (void)sprintf(errmsg,"Time in MAT-file "
                          " '%s' must be monotonically increasing.\n",
                          inportFileName);
        }
    }
    
    return (errmsg[0] != '\0'? errmsg: NULL);   
}/*rt_VerifyTimeMonotone*/


/* Function: rt_CheckMatFileWithStructVar ======================================
 * Abstract:
 * Check the struct format variable 
 * 
 *  
 * Returns:
 *	NULL    : success, variable is valid
 *	non-NULL: error string
 */
const char *rt_CheckMatFileWithStructVar( mxArray *inputTimePtr,
                                          mxArray *mxInportSignalDimensions,
                                          mxArray *mxInportSignalValues,
                                          int inportIdx)
                                      
{     
    static char  errmsg[1024];   
    boolean_T isOk; 
    size_t  numDims, numDimsValues; 
    size_t  numOfTimePoints, numTimePts;   
       
    const int *dimsValues;    
    double *dims; 

    errmsg[0] = '\0'; /* assume success */

    isOk = false;
    numDims = 0;  

    numDimsValues    = mxGetNumberOfDimensions(mxInportSignalValues); 
    dimsValues       = mxGetDimensions(mxInportSignalValues); 

    numTimePts = (numDimsValues == 2) ? 
        mxGetM(mxInportSignalValues) : (size_t)dimsValues[numDimsValues-1];
   
    numOfTimePoints = mxGetNumberOfElements(inputTimePtr);
    
    /* 1. Signal dimension field match signal value dimension, if the field 
     * exists 
     */
     
    if (mxInportSignalDimensions != NULL) {
        /* for signal, there is a field called dimensions. check how many 
         * element in the dimension matrix
         */
        
        numDims = mxGetNumberOfElements(mxInportSignalDimensions);    
        
        /* the dimension is either a scaler or a 1X2 vector */
	
        isOk = mxIsIntVector(mxInportSignalDimensions) && 
            (numDims == 1 || numDims == 2);
        
	if (isOk) {           
            /* signal dimension value */           
            dims = mxGetPr(mxInportSignalDimensions);   
            if(numDims == 1) {
                /*if signal dimension is 1 then the signal values must be a 
                 * vector TxV and the signal dimension value = V.              
                 */
                isOk    = numDimsValues == 2 && dimsValues[1] == dims[0];
            } else { /* numDims == 2 */
                isOk = (numDimsValues == 3  || numDimsValues == 2) &&
		    (dimsValues[0] == dims[0]) && (dimsValues[1] == dims[1]);
            }
	}
    } else { 
        /* If  no dimension field  */
        
        isOk    = (numDimsValues == 2 || numDimsValues == 3);
    }
    
    if (!isOk) {
        (void)sprintf(errmsg, "Invalid structure-format variable specified in " 
                      " the matfile.  The structure's 'dimensions' field must "
                      " be a scalar or a vector with 2 elements. In addition, "
                      " this field must be compatible with the dimensions of "
                      " input signal stored in the 'values' field.\n");
        goto EXIT_POINT;
    }
                    
    /* 2. Time and data have the same number of rows if have time */
    
    if(numOfTimePoints > 0){
        if(numOfTimePoints != numTimePts){
            (void)sprintf(errmsg, "Invalid structure-format variable. Time "
                          " and data values must have the same number of rows.\n");
            goto EXIT_POINT;
        }
    }
 
    /* 3. Signal value dimension must equal to inport file dimension  
     * numDimsValues must >= 2 
     */
    
    if (numDimsValues == 2){
        if (dimsValues[1] != gblInportDims[inportIdx*2]){
            (void)sprintf(errmsg,"Dimension of signal #%d from MAT-file not " 
                          "equal the dimension of the corresponding inports "
                          "specified in the model.\n", inportIdx+1);
            goto EXIT_POINT;                             
        }
    }
    
    if (numDimsValues == 3){
      if (dimsValues[0] != gblInportDims[inportIdx*2] 
            ||dimsValues[1] != gblInportDims[inportIdx*2+1]){
            (void)sprintf(errmsg,"Dimension of signal #%d from MAT-file not " 
                          "equal the dimension of the corresponding inports "
                          "in specified in the model.\n", inportIdx+1);
            goto EXIT_POINT;                              
        }
    }
    
    /* 4. Signal data type must equal the inport data type */
    {
        int_T            dTypeID = gblInportDataTypeIdx[inportIdx]; 
        mxClassID  mxIDfromModel = rt_GetMxIdFromDTypeIdForRSim(dTypeID);
        mxClassID  mxIDfromFile  = mxGetClassID(mxInportSignalValues);
        
        if((mxIDfromModel != mxUNKNOWN_CLASS) &&
           (mxIDfromModel != mxIDfromFile)){
            (void)sprintf(errmsg,"Data type of signal #%d from MAT-file not "
                          "equal the Data type of the corresponding inports "
                          "specified in the model.\n", inportIdx+1);
            goto EXIT_POINT;                              
        }
    }
    
    /* 5. Check for port complexity mismatch */
    {
        boolean_T isComplexFromModel = mxIsComplex(mxInportSignalValues);
        boolean_T isComplexFromFile  = (bool)gblInportComplex[inportIdx];
        
        if(isComplexFromModel != isComplexFromFile){
            printf("*** Warning: Signal complexity of signal #%d from MAT-file "
                   "is not as same as the signal complexity of the corresponding inports. "
                   "The complex part is ignored. ***\n ", inportIdx+1);
        }
    }
    
EXIT_POINT:
    return (errmsg[0] != '\0'? errmsg: NULL);
} /*rt_CheckMatFileWithStructVar */


/* Function: rt_CheckMatFileStructTimeField ======================================
 * Abstract:
 * Check the struct time field
 * 
 *  
 * Returns:
 *	NULL    : success, Time field is valid
 *	non-NULL: error string
 */
const char* rt_CheckMatFileStructTimeField( mxArray *inputTimePtr, 
                                            int_T matFileDataFormat,
                                            int_T inportIdx,
                                            const char* inportFileName)
{                                          
    static char errmsg[1024];
    const char* result;
    int_T i;
    double *timeDataPtr;
    int_T numOfTimePoints;

    int timeDataPtrOffset = 1;
    
    if(inputTimePtr!= NULL){
        numOfTimePoints = mxGetNumberOfElements(inputTimePtr);
        /* if time vector is empty, must be discrete sample time 
         * interpolation must be set to off                    */
        
        if(numOfTimePoints == 0){
            if(matFileDataFormat == SINGLEVAR_STRUCT){
                for (i = 0 ; i<gblNumRootInportBlks; i++){
                    if (gblInportInterpoFlag[i]!= 0) {
                        (void)sprintf(errmsg, "The MAT-file %s does not specify "
                                      "any time values and interpolation flag of "
                                      "all root inport blocks must set to off.\n", 
                                      inportFileName );
                        goto EXIT_POINT; 
                    }                        
                
                    if (gblInportContinuous[i] == 1) {
                        (void)sprintf(errmsg, "Invalid sample time for inport "
                                      "blocks. Since the variable in the MAT "
                                      "file %s does not specify any time values, "
                                      "the sample time of all root inport blocks "
                                      "must be discrete.\n", inportFileName);
                        goto EXIT_POINT;       
                    }
                }
            }else{
                if (gblInportInterpoFlag[inportIdx]!= 0){
                    (void)sprintf(errmsg, "Invalid interpolation flag for inport %d. "
                                  "Since the variable in MAT-file %s corresponding "
                                  "to this inport does not specify any time values,"
                                  "the interpolation flag  must set to off.\n", inportIdx+1, 
                                  inportFileName);
                    goto EXIT_POINT; 
                }
                
                if (gblInportContinuous[inportIdx] == 1) {
                    (void)sprintf(errmsg, "Invalid sample time for inport %d. "
                                  "Since the variable in MAT-file %s corresponding "
                                  "to the inport does not specify any time values,"
                                  "the sample time must be discrete.\n", inportIdx+1,
                                  inportFileName);
                    goto EXIT_POINT; 
                }   
            }               
        }else{
            timeDataPtr = mxGetPr(inputTimePtr);               
            result = rt_VerifyTimeMonotone(timeDataPtr, 
                                           numOfTimePoints, 
                                           timeDataPtrOffset, 
                                           inportFileName);
            if(result !=NULL){
                (void)strcpy(errmsg, result);
                goto EXIT_POINT;
            }
        }
    }else{
        /* time field is not present, error out*/
        (void)sprintf(errmsg, "Invalid structure-format variable specified as external "
                      "input in MAT-file %s. The variable must include 'time' and 'signals' fields. "
                      "Also, the 'signals' field must be a structure with a 'values' field.\n", 
                      inportFileName);
        goto EXIT_POINT;        
    }   

EXIT_POINT:
    return (errmsg[0] != '\0'? errmsg: NULL);
}/*  rt_CheckMatFileStructTimeField */


/* Function: rt_VerifyInportsMatFile ======================================
 * Abstract:
 * Verify the MAT-file to drive the inport block 
 * 
 *  
 * Returns:
 *	NULL    : success, file is valid
 *	non-NULL: error string
 */

const char* rt_VerifyInportsMatFile(int matFileDataFormat,
                                    const char* inportFileName )
{   
    static char  errmsg[1024];
    const char *result; 
    MATFile *pmat; 
    
    int_T numOfTimePoints;
    const char  *inportVariableName;   

    const double *timeDataPtr;
    mxArray* inputTimePtr = NULL;
    mxArray* mxInputPtr;

    /* inport structure signals */
    mxArray *mxInportSignal = NULL;
    /* signal dimension array */
    mxArray *mxInportSignalDimensions;  
    /* signal value array */
    mxArray *mxInportSignalValues;      
     
    int i;
    int inIdx; 
    int inportIdx;
    int valuesFieldIdx;

    int_T timeDataPtrOffset = 1;

    pmat = matOpen(inportFileName,"rb"); 
    mxInputPtr = matGetNextVariable(pmat, &inportVariableName);

    errmsg[0] = '\0'; /* assume success */

    if (matFileDataFormat == SINGLEVAR_MATRIX)
    {
        /*
         * 1. TU matrix, data type double,  notice that the matrix format 
         * 
         *  nCols = gblNumRootInputSignals + 1, nRows = numOfTimePoints.
         */  

        int nrows = mxGetM(mxInputPtr);
        int ncols = mxGetN(mxInputPtr);    
        mxClassID  mxIDfromFile  = mxGetClassID(mxInputPtr);

        if( mxIDfromFile != rt_GetMxIdFromDTypeId(SS_DOUBLE)){
            (void)sprintf(errmsg,"Data type of matrix variable from "
                          "MAT-file %s is not double!\n" , inportFileName);
            goto EXIT_POINT;
        }

        if ( ncols < 2 ) {
            (void)sprintf(errmsg, "Matrix variable from MAT-file "
                          "'%s' must contain at least 2 columns.\n",inportFileName);
            goto EXIT_POINT;
        }
        
        if ( ncols != gblNumModelInputs + 1) {
            (void)sprintf(errmsg,"The number of columns of matrix variable from "
                          "MAT-file '%s' must equal %d, which is the number of "
                          "root inports plus 1.\n ", inportFileName, 
                          gblNumModelInputs + 1);
            goto EXIT_POINT;
        }
        
        for(i=0; i < gblNumRootInportBlks; i++){
            /* check complexity, only real */         
            if (gblInportComplex[i] == 1) {
                printf("*** Warning: Signal type of matrix variable from "
                       "MAT-file %s can only be real while signal type "
                       "of inport %d is set to complex. The imaginary part " 
                       "is ignored. ***\n",inportFileName, i);
            }
            
            /* check data type, only double */         
            if (gblInportDataTypeIdx[i] != SS_DOUBLE) {  
                (void)sprintf(errmsg,"Data type of matrix variable from "
                              "MAT-file %s can only be double while data type of "
                              "inport %d is not set to double.\n", inportFileName, i);
                goto EXIT_POINT;
            }
            
            /* Check dimension, only vector is supported */
            
            if(gblInportDims[2*i + 1] != 1){
                (void)sprintf(errmsg," The dimensions of inport %d require that the "
                              "input variable must be a structure, while the variable "
                              "in the file %s is in TU matrix format. This format can "
                              "only be used if all the input signals are real, double "
                              "vectors.\n", i, inportFileName);
                goto EXIT_POINT; 
            }
            
        }
        
        timeDataPtr = mxGetPr(mxInputPtr);
        
        numOfTimePoints     = nrows; 
        
        result = rt_VerifyTimeMonotone(timeDataPtr, 
                                       numOfTimePoints,
                                       timeDataPtrOffset,
                                       inportFileName);    
        if(result !=NULL){
            (void)strcpy(errmsg, result);
            goto EXIT_POINT;
        }
        return NULL;
        
    }else if (matFileDataFormat == SINGLEVAR_STRUCT) {
        
        /*
         * Verify that the time vector is monotonically increasing.
         * only one time vector is need to be checked.
         * Check of time value is given, if not interpolation must be off
         * and sample time must be discrete
         */
        
        int_T  numInportSignals;
        inputTimePtr  = mxGetField(mxInputPtr, 0, "time");
        
        result = rt_CheckMatFileStructTimeField(inputTimePtr, 
                                                matFileDataFormat,
                                                0, 
                                                inportFileName);
        if(result != NULL){
            (void)strcpy(errmsg, result);
            goto EXIT_POINT;
        }

        mxInportSignal = mxGetField(mxInputPtr, 0, "signals");
        
        if(mxInportSignal == NULL){
            (void)sprintf(errmsg,"Variable %s in MAT-file '%s' contains no 'signals' field ",                        
                          inportVariableName, inportFileName);
            goto EXIT_POINT;
        }

        numInportSignals   = mxGetN(mxInportSignal);
        
        if(numInportSignals != gblNumRootInportBlks){
            (void)sprintf(errmsg,"MAT-file '%s' contains %d signals while model "
                          " contain %d inports, they must be equal.\n ",
                          inportFileName, numInportSignals, gblNumRootInportBlks);
            goto EXIT_POINT;
        }
    }        
    /*
     * check each signal's dimension information, data type information  
     */
    
    for (inportIdx = 0; inportIdx<gblNumRootInportBlks; inportIdx++){
    
        if(matFileDataFormat == MULTIPLEVAR_STRUCT){           
            inIdx = 0; 
            mxInportSignal = mxGetField(mxInputPtr, 0, "signals"); 
            if(mxInportSignal == NULL){
                (void)sprintf(errmsg,"Variable '%s' in  MAT-file '%s' contains no 'signals' field ",                        
                              inportVariableName, inportFileName);
                goto EXIT_POINT;
        }
            inputTimePtr   = mxGetField(mxInputPtr, 0, "time");
        }else{
            inIdx = inportIdx; 
        }
        
        mxInportSignalDimensions = mxGetField(mxInportSignal, inIdx,
                                              "dimensions"); 

        valuesFieldIdx = mxGetFieldNumber(mxInportSignal, "values");
        if ( valuesFieldIdx == -1) {
            (void)sprintf(errmsg,"External inputs structure must contain 'signals.values' field");
            goto EXIT_POINT;
        }
        mxInportSignalValues = mxGetFieldByNumber(mxInportSignal, inIdx, 
                                                  valuesFieldIdx);
        if(mxInportSignalValues == NULL){
            (void)sprintf(errmsg, "The data type of the 'values' field of 'signals' structure for the input port '%d' is not supported in rapid accelerator", 
                          (inportIdx+1));
            goto EXIT_POINT;
        }
        
        if(matFileDataFormat == MULTIPLEVAR_STRUCT){
            result = rt_CheckMatFileStructTimeField(inputTimePtr, 
                                                    matFileDataFormat,
                                                    inportIdx,
                                                    inportFileName);           
            if(result != NULL){
                (void)strcpy(errmsg, result);
                goto EXIT_POINT;
            } 
        }
        
        result = rt_CheckMatFileWithStructVar(  inputTimePtr, 
                                                mxInportSignalDimensions,
                                                mxInportSignalValues,
                                                inportIdx);
        if(result != NULL){
            (void)strcpy(errmsg, result);
            goto EXIT_POINT;
        }

        if(matFileDataFormat == MULTIPLEVAR_STRUCT){
           mxInputPtr = matGetNextVariable(pmat, &inportVariableName); 
        } 
    }

EXIT_POINT:
    
    if (pmat!=NULL) {
        matClose(pmat); pmat= NULL;
    }
    
    if (mxInputPtr != NULL) {
        mxDestroyArray(mxInputPtr);
    }
    
    return (errmsg[0] != '\0'? errmsg: NULL);   

}/*  const char* rt_VerifyInportsMatFile */


/* Function:rt_ConvertInportsMatDatatoTUtable==================================
 * Abstract:
 * Convert the MatData to TUtable in memory
 * 
 *  
 * Returns:
 *	NULL    : success, Data is in memory
 *	non-NULL: error string
 */

const char* rt_ConvertInportsMatDatatoTUtable(int_T matFileDataFormat,
                                              const char* inportFileName
                                              )
{
    /* if we get here , the file is valid, need to read in the file to 
     * inportUTtable. Each inport has its own tu table, it is because
     * we assume the user will change the MAT-file format after the 
     * code is generated and don't want to recompile 
     */
    
    static char  errmsg[1024];
       
    char *matDataRe = NULL;
    char *matDataIm = NULL;
    
    int_T  inIdx;
    int_T  timeDataPtrOffset = 1;
    size_t numOfTimePoints = 0; /* invalid value */
    int_T  portWidth;
    int_T  inportIdx;
    size_t elementSize = 0; /* invalid value */
  
    mxArray *mxInportSignal;
    const double* timeDataPtr = NULL;       
    double *dataPtr;

    mxArray* inputTimePtr;
    mxArray* mxInportSignalValues = NULL;
    
  
    MATFile *pmat= matOpen(inportFileName,"rb");   
    mxArray *mxInputPtr = matGetNextVariable(pmat, NULL); 

    errmsg[0] = '\0'; /* assume success */
     
    /* allocate memeory for tu table */
    
    gblInportTUtables = (rtInportTUtable*)
        malloc(sizeof(rtInportTUtable)*gblNumRootInportBlks);
    
    if (gblInportTUtables == NULL) {
        (void)sprintf(errmsg,"Memory allocation error"); 
        goto EXIT_POINT;
    }
    
    for (inportIdx = 0; inportIdx<gblNumRootInportBlks; inportIdx++){

        /* initialize the element of TU table to NULL */
        gblInportTUtables[inportIdx].ur   =NULL;
        gblInportTUtables[inportIdx].ui   =NULL;
        gblInportTUtables[inportIdx].time =NULL; 
        
        portWidth  = gblInportDims[inportIdx*2]*gblInportDims[inportIdx*2 + 1]; 
            
        if(matFileDataFormat == SINGLEVAR_MATRIX){
            
            if(inportIdx == 0){
                numOfTimePoints = mxGetM(mxInputPtr);                
                timeDataPtr  = mxGetPr(mxInputPtr);                                               
                elementSize  = sizeof(double);
                dataPtr = mxGetPr(mxInputPtr); 
                matDataRe = (char*)dataPtr + numOfTimePoints*elementSize;
            }            
            gblInportTUtables[inportIdx].complex = 0;             

        }else {

            int_T numDimsValues;
            const int_T *dimsValues;
            
            mxInportSignal = mxGetField(mxInputPtr, 0 , "signals");
            
            if(matFileDataFormat == MULTIPLEVAR_STRUCT){
                inIdx = 0;
            }else{
                inIdx = inportIdx; 
            }
            
            inputTimePtr  = mxGetField(mxInputPtr, 0, "time");
            mxInportSignalValues = mxGetField(mxInportSignal, inIdx, "values");
            
            timeDataPtr = mxGetPr(inputTimePtr);
            numOfTimePoints = mxGetNumberOfElements(inputTimePtr);

            numDimsValues   = mxGetNumberOfDimensions(mxInportSignalValues);      
            dimsValues      = mxGetDimensions(mxInportSignalValues); 
                
            if(numOfTimePoints ==0){                
                numOfTimePoints = (numDimsValues == 2) ? 
                    mxGetM(mxInportSignalValues) : (size_t)dimsValues[numDimsValues-1];
            }
                              
            gblInportTUtables[inportIdx].complex = 
                (gblInportComplex[inportIdx]!=mxIsComplex(mxInportSignalValues))? 0 : gblInportComplex[inportIdx];  
            elementSize = mxGetElementSize(mxInportSignalValues);  
        }    
        
        /* allocate memory */
        gblInportTUtables[inportIdx].ur = 
            (char*)calloc(numOfTimePoints*portWidth,elementSize);     
        
        if (gblInportTUtables[inportIdx].ur == NULL) {
            (void)sprintf(errmsg,"Memory allocation error"); 
            goto EXIT_POINT;
        }            
        
        if(gblInportTUtables[inportIdx].complex == 1){
            gblInportTUtables[inportIdx].ui = 
                (char*)calloc(numOfTimePoints*portWidth,elementSize);     
            if (gblInportTUtables[inportIdx].ui == NULL) {
                (void)sprintf(errmsg,"Memory allocation error"); 
                goto EXIT_POINT;
            }
        }          
        
        /* steal time ptr to TUtable*/ 
        
        if(timeDataPtr!= NULL)
        {
            if(matFileDataFormat != MULTIPLEVAR_STRUCT)
            {
                if(inportIdx == 0){
                    gblInportTUtables[inportIdx].time = 
                        (double*)calloc(numOfTimePoints*portWidth,sizeof(double));                                 
                }else{
                    gblInportTUtables[inportIdx].time = gblInportTUtables[0].time;
                }
            }else{
                gblInportTUtables[inportIdx].time = 
                    (double*)calloc(numOfTimePoints*portWidth,sizeof(double));   
            }
         
            if (gblInportTUtables[inportIdx].time == NULL) {
                (void)sprintf(errmsg,"Memory allocation error"); 
                goto EXIT_POINT;
            }
        }
                
        /* set TU Table attribute */

        gblInportTUtables[inportIdx].nTimePoints        
            = numOfTimePoints;
        
        gblInportTUtables[inportIdx].uDataType = gblInportDataTypeIdx[inportIdx]; 
         
        gblInportTUtables[inportIdx].currTimeIdx = timeDataPtr ? 0 : -1;  

        /* now we can steal the data from the mxArray and generate tu table */
        
        if(timeDataPtr != NULL){
            if(matFileDataFormat == MULTIPLEVAR_STRUCT){
                (void)memcpy(gblInportTUtables[inportIdx].time, timeDataPtr, 
                                sizeof(double)*numOfTimePoints);
            }else if (matFileDataFormat == SINGLEVAR_STRUCT){
                if(inportIdx==0){
                    (void)memcpy(gblInportTUtables[inportIdx].time, 
                                    timeDataPtr, sizeof(double)*numOfTimePoints);
                }else{
                    gblInportTUtables[inportIdx].time = gblInportTUtables[0].time;
                }
            }else{
                if(inportIdx ==0){
                    size_t i;
                    for (i=0; i<numOfTimePoints; i++) {           
                        gblInportTUtables[inportIdx].time[i] = *timeDataPtr;
                        timeDataPtr  = timeDataPtr + timeDataPtrOffset;
                    }
                    
                }
                
            }
        }
        
        if(matFileDataFormat != SINGLEVAR_MATRIX){
            matDataRe = mxGetData(mxInportSignalValues);    
        }
          
        (void)memcpy(gblInportTUtables[inportIdx].ur, matDataRe, 
                     elementSize*numOfTimePoints*portWidth);
        
        if(gblInportTUtables[inportIdx].complex){                    
            matDataIm = mxGetImagData(mxInportSignalValues);
            (void)memcpy(gblInportTUtables[inportIdx].ui, matDataIm, 
                         elementSize*numOfTimePoints*portWidth);
        }                  
         
        /* for multiple structure, move to next structure */
        if(matFileDataFormat == MULTIPLEVAR_STRUCT){
            mxInputPtr = matGetNextVariable(pmat, NULL); 
        }   
        
        if(matFileDataFormat == SINGLEVAR_MATRIX){
            matDataRe = matDataRe + portWidth*numOfTimePoints*elementSize;
        }
    }   
 
EXIT_POINT:
    
    if (pmat!=NULL) {
        matClose(pmat); pmat = NULL;
    }
    
    if (mxInputPtr != NULL) {
        mxDestroyArray(mxInputPtr);
    }
    
    return (errmsg[0] != '\0'? errmsg: NULL);   
} /* rt_ConvertInportsMatDatatoTUtable */        



/* Function: FreeFNamePairList ================================================
 * Abstract:
 *	Free name pair lists.
 */
static void FreeFNamePairList(FNamePair *list, int_T len)
{
    if (list != NULL) {
        int_T i;
        for (i = 0; i < len; i++) {
            if (list[i].oldName != NULL) free(list[i].oldName);
        }
        free(list);
    }

} /* end FreeFFnameList */

/* Function: ReplaceRtP ========================================================
 * Abstract
 *  Initialize the rtP structure using the parameters from the specified
 *  'paramStructure'.  The 'paramStructure' contains parameter info that was
 *  read from a mat file (see raccel_mat.c/rt_ReadParamStructMatFile).
 */
static const char *ReplaceRtP(const SimStruct *S,
                              const PrmStructData *paramStructure)
{
    int                     i;
    const char              *errStr        = NULL;
    const DTParamInfo       *dtParamInfo   = paramStructure->dtParamInfo;
    int                     nTrans         = paramStructure->nTrans;
    const DataTypeTransInfo *dtInfo        = (const DataTypeTransInfo *)ssGetModelMappingInfo(S);
    DataTypeTransitionTable *dtTable       = dtGetParamDataTypeTrans(dtInfo);
    uint_T                  *dataTypeSizes = dtGetDataTypeSizes(dtInfo);

    for (i=0; i<nTrans; i++) {
        int  dataTransIdx  = dtParamInfo[i].dtTransIdx;
        char *transAddress = dtTransGetAddress(dtTable, dataTransIdx);
        bool complex       = (bool)dtParamInfo[i].complex;
        int  dataType      = dtParamInfo[i].dataType;
        int  dtSize        = (int)dataTypeSizes[dataType];
        int  nEls          = dtParamInfo[i].nEls;
        int  elSize        = dtParamInfo[i].elSize;
        int  nParams       = (elSize*nEls)/dtSize;
        if (!nEls) continue;
        /*
         * Check for consistent element size.  dtParamInfo->elSize is the size
         * as stored in the parameter mat-file.  This should match the size
         * used by the generated code (i.e., stored in the SimStruct).
         */
        if ((dataType <= 13 && elSize != dtSize) ||
            (dataType > 13 && (dtSize % elSize != 0))){
            errStr = "Parameter data type sizes in MAT-file not same "
                "as data type sizes in RTW generated code";
            goto EXIT_POINT;
        }

        if (!complex) {
            (void)memcpy(transAddress,dtParamInfo[i].rVals,nParams*dtSize);
        } else {
            /*
             * Must interleave the real and imaginary parts.  Simulink style.
             */
            int  j;
            char *dst     = transAddress;
            const char *realSrc = (const char *)dtParamInfo[i].rVals;
            const char *imagSrc = (const char *)dtParamInfo[i].iVals;

            for (j=0; j<nParams; j++) {
                /* Copy real part. */
                (void)memcpy(dst,realSrc,dtSize);
                dst     += dtSize;
                realSrc += dtSize;

                /* Copy imag part. */
                (void)memcpy(dst,imagSrc,dtSize);
                dst     += dtSize;
                imagSrc += dtSize;
            }
        }
    }

EXIT_POINT:
    return(errStr);
} /* end ReplaceRtP */


/*==================*
 * Visible routines *
 *==================*/

/* Function: rt_RapidReadFromFileBlockMatFile ============================================

 *
 * Abstract:
 *      This function opens a "fromfile" matfile containing a TU matrix.
 *      The first row of the TU matrix contains a time vector, while
 *      successive rows contain one or more U vectors. This function
 *      expects to find one and only one matrix in the
 *      matfile which must be named "TU".
 *
 *      originalWidth    = only the number of U channels (minimum is 1)
 *      nptsPerSignal    = the length of the T vector.
 *      nptsTotal        = total number of point in entire TU matrix.
 *                         npoints equals: nptsPerChannel * (nchannels + 1)
 *
 * Returns:
 *	NULL    : success
 *      non-NULL: error message
 */
const char *rt_RapidReadFromFileBlockMatFile(const char *origFileName,
                                   int originalWidth,
                                   FrFInfo * frFInfo)
{
    static char  errmsg[1024];
    MATFile      *pmat;
    mxArray      *tuData_mxArray_ptr = NULL;
    const double *matData;
    size_t       nbytes;
    int          nrows, ncols;
    int          rowIdx, colIdx;
    const char   *matFile;

    errmsg[0] = '\0'; /* assume success */

    /******************************************************************
     * Remap the "original" MAT-filename if told to do by user via a *
     * -f command line switch.                                        *
     ******************************************************************/
    {
        int_T i;

        frFInfo->origFileName  = origFileName;
        frFInfo->originalWidth = originalWidth;
        frFInfo->newFileName   = origFileName; /* assume */

        for (i=0; i<gblNumFrFiles; i++) {
            if (gblFrFNamepair[i].newName != NULL && \
                strcmp(origFileName, gblFrFNamepair[i].oldName)==0) {
                frFInfo->newFileName = gblFrFNamepair[i].newName; /* remap */
                gblFrFNamepair[i].remapped = 1;
                break;
            }
        }
    }

    if ((pmat=matOpen(matFile=frFInfo->newFileName,"rb")) == NULL) {
        (void)sprintf(errmsg,"could not open MAT-file '%s' containing "
                      "From File Block data", matFile);
        goto EXIT_POINT;
    }

    if ( (tuData_mxArray_ptr=matGetNextVariable(pmat,NULL)) == NULL) {
        (void)sprintf(errmsg,"could not locate a variable in MAT-file '%s'",
                      matFile);
        goto EXIT_POINT;
    }

    nrows=mxGetM(tuData_mxArray_ptr);
    if ( nrows<2 ) {
        (void)sprintf(errmsg,"\"From File\" matrix variable from MAT-file "
                      "'%s' must contain at least 2 rows", matFile);
        goto EXIT_POINT;
    }

    ncols=mxGetN(tuData_mxArray_ptr);

    frFInfo->nptsPerSignal = ncols;
    frFInfo->nptsTotal     = nrows * ncols;

    /* Don't count Time as part of output vector width */
    if (frFInfo->originalWidth != nrows) {
        /* Note, origWidth is determined by fromfile.tlc */
        (void)sprintf(errmsg,"\"From File\" number of rows in MAT-file "
                      "'%s' must match original number of rows", matFile);
        goto EXIT_POINT;
    }

    matData = mxGetPr(tuData_mxArray_ptr);

    /*
     * Verify that the time vector is monotonically increasing.
     */
    {
        int i;
        for (i=1; i<ncols; i++) {
            if (matData[i*nrows] < matData[(i-1)*nrows]) {
                (void)sprintf(errmsg,"Time in \"From File\" MAT-file "
                              "'%s' must be monotonically increasing",
                              matFile);
                goto EXIT_POINT;
            }
        }
    }

    /*
     * It is necessary to have the same number of input signals as
     * in the original model. It is NOT necessary for the signals to
     * have the same signal length as in the original model. They
     * can be substantially larger if desired.
     */
    nbytes = (size_t)(nrows * ncols * (size_t)sizeof(double));

    if ((frFInfo->tuDataMatrix = (double*)malloc(nbytes)) == NULL) {
        (void)sprintf(errmsg,"memory allocation error "
                      "(rt_RapidReadFromFileBlockMatFile %s)", matFile);
        goto EXIT_POINT;
    }

    /* Copy and transpose data into "tuDataMatrix" */
    for (rowIdx=0; rowIdx<frFInfo->originalWidth; rowIdx++) {
        for (colIdx=0; colIdx<frFInfo->nptsPerSignal; colIdx++) {
            frFInfo->tuDataMatrix[colIdx + rowIdx*frFInfo->nptsPerSignal] =
                matData[rowIdx + colIdx*frFInfo->originalWidth];
        }
    }


EXIT_POINT:

    if (pmat!=NULL) {
        matClose(pmat);
        pmat = NULL;
    }

    if (tuData_mxArray_ptr != NULL) {
        mxDestroyArray(tuData_mxArray_ptr);
    }

    return (errmsg[0] != '\0'? errmsg: NULL);

} /* end rt_RapidReadFromFileBlockMatFile */


/* Function: rt_RapidReadInportsMatFile ============================================
 *
 * Abstract:
 *      This function opens a "Inport matfile" and read it to InportTUTable    
 *      
 *      
 *      
 * Returns:
 *	NULL    : success
 *      non-NULL: error message
 */
const char *rt_RapidReadInportsMatFile(const char* inportFileName, int* matFileFormat)              
{
    int i;

    static char  errmsg[1024];
    const  char* result;
    bool   isStruct = true;     

    /* variable name defined in the config panel */
    const char  *inportVariableName;   
    const char   *matFile;
    
    int_T numVarInMatFile = 0;      
    int_T fileDataFormat = -1; 
    
    MATFile      *pmat = NULL;
    mxArray      *inportData_mxArray_ptr = NULL;
   
    errmsg[0] = '\0'; /* assume success */
    
    /* no root inports return */
    if(gblNumRootInportBlks == 0){
        return NULL;
    }else{
        if(inportFileName == NULL){
            printf("*** Warning: -i command is not used and there are "
                   "inports block in the model. All inports will be "
                   "set to zero. ***\n ");
            goto EXIT_POINT;
        }
    }
    
    if ((pmat=matOpen(matFile = inportFileName,"rb")) == NULL) {
        (void)sprintf(errmsg,"could not open MAT-file '%s' containing "
                      "Inport Block data", matFile);
        goto EXIT_POINT;
    }   
    
    while((inportData_mxArray_ptr = 
            matGetNextVariable(pmat, &inportVariableName))!=NULL)
    {
        numVarInMatFile ++;
        isStruct = isStruct &&  mxIsStruct(inportData_mxArray_ptr); 
    }
    matClose(pmat);
    pmat = NULL;
    
    if (numVarInMatFile ==0) {
        (void)sprintf(errmsg,"could not locate any variables in MAT-file '%s'",
                      matFile);
        goto EXIT_POINT;
    }
    
    /* We support three types of MAT-file */
    
    
    pmat = matOpen(matFile = inportFileName,"rb");
    inportData_mxArray_ptr = matGetNextVariable(pmat, &inportVariableName);
    
    if (numVarInMatFile == 1) {     

        if(!isStruct){
            /* must be matrix */
            fileDataFormat = SINGLEVAR_MATRIX;
        }else{
            /*
             * 2. One variable (structure) contains all inport signals 
             */
            fileDataFormat = SINGLEVAR_STRUCT;
        }     
        
    }else{ /* Multi-variable structure */
        
        if(isStruct) {
            /*
             * 3. NumOfRootInports variables(structures), each map to one inport  
             */ 
            fileDataFormat = MULTIPLEVAR_STRUCT;
            if(numVarInMatFile < gblNumRootInportBlks){
                (void)sprintf(errmsg,"MAT-file '%s' contains %d signals "
                              "while model contain %d inports. ",
                              inportFileName, numVarInMatFile, gblNumRootInportBlks);
                goto EXIT_POINT;               
            }
            /*
             * 4. Show the variables and the coresponding inport  
             */
            printf("** Variables in  MAT-file '%s' are to "
                   "be read to the inport blocks  as follows:\n", inportFileName);
            
            for ( i = 1; i<gblNumRootInportBlks+1; i++)
            {
                printf("    %s ----> In%d\n", inportVariableName, i);
                inportData_mxArray_ptr = 
                    matGetNextVariable(pmat, &inportVariableName);
            }            
            printf("\n");
            printf("If this is not correct, you can change the inport port "
                   "index in the model to match the variables in the MAT-file "
                   "and the coresponding inport blocks and recompile or use "
                   "save -append to create the MAT-file that has the order you "
                   "want.\n"); 
        }else{
            (void)sprintf(errmsg,"Could not support data format in '%s'. "
                          "Data must be in either single variable in TU matrix format "
                          "or single/multiple variables in structure format", 
                          matFile);
            goto EXIT_POINT;
        } 
    }

    matClose(pmat);
    pmat = NULL;

    *matFileFormat = fileDataFormat; 
    result = rt_VerifyInportsMatFile(fileDataFormat, inportFileName);
                                         
    if (result != NULL){
        (void)strcpy(errmsg, result);
        goto EXIT_POINT;
    }
    
    /* reach here, data file is valid, steal data to TU table*/
    result  = rt_ConvertInportsMatDatatoTUtable(fileDataFormat,
                                                inportFileName
                                                );
    if (result != NULL){
        (void)strcpy(errmsg, result);
        goto EXIT_POINT;
    }
    /* Reach here, data is successfully loaded */
    printf(" *** %s is successfully loaded! ***\n", inportFileName);

EXIT_POINT:
    
    if (pmat!=NULL) {
        matClose(pmat); pmat = NULL;
    }
    
    if (inportData_mxArray_ptr != NULL) {
        mxDestroyArray(inportData_mxArray_ptr);
    }

    if( errmsg[0] != '\0'){
        gblInportFileName = NULL;
        return errmsg;
    }else{
        return NULL;
    }
} /* end rt_RapidReadInportsMatFile */


/* Function:  Interpolate_Datatype================================
 * Abstract:
 *      Performs Lagrange interpolation on a pair of data values of
 *      specified data type.
 *
 */

void rt_Interpolate_Datatype(void   *x1, void   *x2, void   *yout,
    				    real_T t,   real_T t1,  real_T t2,
                                    int    outputDType)
{
    real_T  out;
    real_T  f1 = (t2 - t) / (t2 - t1);
    real_T  f2 = 1.0 - f1;

    switch(outputDType){

      case SS_DOUBLE:
  	  out = Interpolate(*(real_T *)x1, *(real_T *)x2, f1, f2);
          *(real_T *) yout = out;
          break;

      case SS_SINGLE:
          out = Interpolate(*(real32_T *)x1, *(real32_T *)x2, f1, f2);
          *(real32_T *) yout = (real32_T) out;
          break;

      case SS_INT8:
          out = Interpolate(*(int8_T *)x1,*(int8_T *)x2, f1, f2);

          if (out >= MAX_int8_T) {
	      *(int8_T *)yout = MAX_int8_T;
	  } else if (out <= MIN_int8_T) {
	      *(int8_T *)yout = MIN_int8_T;
	  } else {
	      *(int8_T *)yout = (int8_T)InterpRound(out);
	  }
          break;

      case SS_UINT8:

          out = Interpolate(*(uint8_T *)x1,*(uint8_T *)x2, f1, f2);

	  if (out >= MAX_uint8_T) {
	      *(uint8_T *)yout = MAX_uint8_T;
	  } else if (out <= MIN_uint8_T) {
	      *(uint8_T *)yout = MIN_uint8_T;
	  } else {
	      *(uint8_T *)yout = (uint8_T)InterpRound(out);
	  }

          break;

      case SS_INT16:
          out = Interpolate(*(int16_T *)x1,*(int16_T *)x2, f1, f2);

          if (out >= MAX_int16_T) {
	      *(int16_T *)yout = MAX_int16_T;
	  } else if (out <= MIN_int16_T) {
	      *(int16_T *)yout = MIN_int16_T;
	  } else {
	      *(int16_T *)yout = (int16_T)InterpRound(out);
	  }
          break;

      case SS_UINT16:
          out = Interpolate(*(uint16_T *)x1,*(uint16_T *)x2, f1, f2);

	  if (out >= MAX_uint16_T) {
	      *(uint16_T *)yout = MAX_uint16_T;
	  } else if (out <= MIN_uint16_T) {
	      *(uint16_T *)yout = MIN_uint16_T;
	  } else {
	      *(uint16_T *)yout = (uint16_T)InterpRound(out);
	  }

          break;

      case SS_INT32:
          out = Interpolate(*(int32_T *)x1,*(int32_T *)x2, f1, f2);

          if (out >= MAX_int32_T) {
	      *(int32_T *)yout = MAX_int32_T;
	  } else if (out <= MIN_int32_T) {
	      *(int32_T *)yout = MIN_int32_T;
	  } else {
	      *(int32_T *)yout = (int32_T)InterpRound(out);
	  }

          break;

      case SS_UINT32:
          out = Interpolate(*(uint32_T *)x1,*(uint32_T *)x2, f1, f2);

	  if (out >= MAX_uint32_T) {
	      *(uint32_T *)yout = MAX_uint32_T;
	  } else if (out <= MIN_uint32_T) {
	      *(uint32_T *)yout = MIN_uint32_T;
	  } else {
	      *(uint32_T *)yout = (uint32_T)InterpRound(out);
	  }

          break;

      case SS_BOOLEAN:
          /*
           * For Boolean interpolation amounts to choosing the point that
           * is closest in time.
           */
          *(boolean_T *) yout = (fabs(t-t1) < fabs(t-t2)) ? (*(boolean_T *)x1) :
                                                            (*(boolean_T *)x2);
          break;

      default:  
          break;
    }
}    /* end rt_Interpolate_Datatype */


/* Function:  rt_getTimeIdx ================================
 * Abstract:
 *      Given a time array and time, get time index so
 *      that timePtr[currTimeIdx]<t< timePtr[currTimeIdx+1]
 *      Note that if interpolation is off, when t is 
 *      larger than last time point, use last time point. 
 */


int_T rt_getTimeIdx(real_T *timePtr, real_T t, int_T numTimePoints, 
                    int_T preTimeIdx, boolean_T interp)
{
    int_T currTimeIdx   = preTimeIdx;
    
    if(numTimePoints<2){
        return timePtr ? currTimeIdx : currTimeIdx+1;
    }

    /* For structure has no time value, when reach last point
     *  output should be 0. return currTimeIdx = -1
     */
    if(timePtr == NULL){
        if(currTimeIdx == numTimePoints - 1 ) currTimeIdx = -2; 
        return currTimeIdx +1;
    }
    
    /* when interpolation is off, when t< t0 or t>tfinal, set
     * currTimeIdx = -1 to output 0 
     */

    if ((fabs(t - timePtr[0]) < rapid_eps(t))) {
        currTimeIdx = 0;
    }else if (t < timePtr[0]){
        if(interp) {
            currTimeIdx = 0;
        }else{
            currTimeIdx = -1;
        }
    }else if ((t > timePtr[numTimePoints - 1]) || 
              (fabs(t - timePtr[numTimePoints - 1]) < rapid_eps(t))) {
        /* when reach last time point, if interpolation is on
         * use last two points to extrapolate, other wise retrun 
         -1 */

        if(interp){
            currTimeIdx = numTimePoints - 2;
        }else{
            if(fabs(t - timePtr[numTimePoints - 1]) < rapid_eps(t)){
                currTimeIdx = numTimePoints - 1;
            }
            else{
                currTimeIdx = -1; 
            }
        }
    } else {
        if (t < timePtr[currTimeIdx]) {
            while (t < timePtr[currTimeIdx]) {
                currTimeIdx--;
            }
        } else {
            while (t >= timePtr[currTimeIdx + 1]) {
                currTimeIdx++;
            }
        }
    }
    return currTimeIdx;
}     /* end rt_getTimeIdx */




/* Function: rt_RapidFreeGbls ========================================================
 * Abstract:
 *	Free global memory prior to exit
 
*/
void rt_RapidFreeGbls(int matFileFormat)
{
    FreeFNamePairList(gblToFNamepair, gblNumToFiles);
    FreeFNamePairList(gblFrFNamepair, gblNumFrFiles);
    
    if(gblNumRootInportBlks>0){
        int i;
        if (gblInportTUtables!= NULL){
            for(i=0; i< gblNumRootInportBlks; i++){
                
                if(gblInportTUtables[i].time != NULL){
                    if(matFileFormat == 2){
                        free(gblInportTUtables[i].time);
                    }else{
                        if(i==0){
                            free(gblInportTUtables[0].time);
                        }
                    }                    
                    gblInportTUtables[i].time = NULL;
                }
                
                if(gblInportTUtables[i].ur != (char*)NULL){
                    free(gblInportTUtables[i].ur);
                    gblInportTUtables[i].ur = NULL;
                }
                if(gblInportTUtables[i].ui != (char*)NULL){
                    free(gblInportTUtables[i].ui);
                    gblInportTUtables[i].ui = NULL;
                }          
            }
            free(gblInportTUtables);
        }    
    }
} /* end rt_RapidFreeGbls */


/* Function: rt_RapidCheckRemappings ==================================================
 * Abstract:
 *	Verify that the FromFile switches were used
 *
 * Returns:
 *	NULL     - success
 *	non-NULL - error message
 */
const char *rt_RapidCheckRemappings(void)
{
    int_T i;

    for (i = 0; i < gblNumFrFiles; i++) {
        if (gblFrFNamepair[i].oldName != NULL && gblFrFNamepair[i].remapped==0){
            return("one or more -f switches from file names do not exist in "
                   "the model");
        }
    }

    for (i = 0; i < gblNumToFiles; i++) {
        if (gblToFNamepair[i].oldName != NULL && gblToFNamepair[i].remapped==0){
            return("one or more -t switches from file names do not exist in "
                   "the model");
        }
    }
    return(NULL);

} /* end rt_RapidCheckRemappings */



/* Function: rt_RapidReadMatFileAndUpdateParams ========================================
 *
 */
void rt_RapidReadMatFileAndUpdateParams(const SimStruct *S)
{
    const char*    result         = NULL;
    PrmStructData* paramStructure = NULL;

    if (gblParamFilename == NULL) goto EXIT_POINT;

    result = rt_ReadParamStructMatFile(&paramStructure, gblParamCellIndex);
    if (result != NULL) goto EXIT_POINT;

    /* be sure checksums all match */
    if (paramStructure->checksum[0] != ssGetChecksum0(S) ||
        paramStructure->checksum[1] != ssGetChecksum1(S) ||
        paramStructure->checksum[2] != ssGetChecksum2(S) ||
        paramStructure->checksum[3] != ssGetChecksum3(S) ) {
        result = "model checksum mismatch - incorrect parameter data "
            "specified";
        goto EXIT_POINT;
    }

    /* Replace the rtP structure */
    result = ReplaceRtP(S, paramStructure);
    if (result != NULL) goto EXIT_POINT;

  EXIT_POINT:
    if (paramStructure != NULL) {
        rt_FreeParamStructs(paramStructure);
    }
    if (result) ssSetErrorStatus(S, result);
    return;

} /* rt_RapidReadMatFileAndUpdateParams */



/* EOF rapid_utils.c */
