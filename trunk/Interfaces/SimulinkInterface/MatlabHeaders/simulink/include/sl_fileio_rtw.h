#ifdef SUPPORTS_PRAGMA_ONCE
# pragma once
#endif
#ifndef sl_fileio_rtw_h
#define sl_fileio_rtw_h
/*
 *
 * Copyright 2008-2012 The MathWorks, Inc.
 *
 * This header is the interface to the sl_fileio module. It is used by the
 * RTW-RSim and Rapid Accelerator targets.
 *
 * 
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

extern const char *rtwH5LoggingCreateInstance(
    const char     *fileName, 
    int            errorXMLMode,
    const char     *objName,
    const char     *sigName,
    const char     *interpolationName,
    const char     *className,
    const int       nDims,
    const int      *dims,
    const int       isComplex,
    const int       decimation,
    const int       formatInt,
    const int       isFixedPoint,
    const int       dataTypeMode,
    const int       isSigned,
    const int       wordLength,
    const double    slopeAdjustmentFactor,
    const int       fixedExponent,
    const double    bias,
    void          **outH5logging);

extern const char *rtwH5LoggingDestroyInstance(
    int  errorXMLMode,
    void *h5logging);

extern const char *rtwH5LoggingWrite(
    int          errorXMLMode,
    void         *h5logging,
    const double  t,
    const void   *u);

extern const char *rtwH5LoggingCollectionCreateInstance(
    int    errorXMLMode,
    void **outH5LoggingCollection
    );

extern const char *rtwH5LoggingCollectionDestroyInstance(
    int   errorXMLMode,
    void *h5LoggingCollection);

extern const char *rtwH5LoggingCollectionAddElement(
    int           errorXMLMode,
    void         *h5LoggingCollection,
    const char   *fileName,
    const char   *mtxName,
    const int     leafIdx,
    const char   *signalName,
    const char   *interpolationName,
    const char   *className,
    const int     nDims,
    const int    *dims,
    const int     isComplex,
    const int     decimation,
    const int     formatInt,
    const int     isFixedPoint,
    const int     dataTypeMode,
    const int     isSigned,
    const int     wordLength,
    const double  slopeAdjustmentFactor,
    const int     fixedExponent,
    const double  bias
    );

extern const char *rtwH5LoggingCollectionWrite(
    int           errorXMLMode,
    void         *h5LoggingCollection,
    const size_t  elementId,
    const double  t,
    const void   *u
    );

extern const char *rtwSetMcosObjName(
    const char *fileName,
    int        errorXMLMode,
    const char *objName
    );

extern const char *rtwMATFileLoaderCreateInstance(
    const char  *fileName,
    int          errorXMLMode,
    const int    extrapolationBeforeFirstDataPointInt,
    const int    interpolationWithinTimeRangeInt,
    const int    extrapolationAfterLastDataPointInt,
    const unsigned char *ground,
    const int    iZeroCrossingSwitch,
    int          fxpDiagnosticOverflow,
    const char  *fxpBlockPath,
    void        **outMATFileLoader
    );

extern const char *rtwMATFileLoaderDestroyInstance(
    int  errorXMLMode,
    void *pMATFileLoader
    );

extern const char *rtwMATFileLoaderGetOutput(
    int            errorXMLMode,
    void          *pMATFileLoader,
    const double   t,
    const int      iMajorTimeStep,
    void         **outOutputValue
    );

extern const char *rtwMATFileLoaderGetZeroCrossingSignal(
    void         *pMATFileLoader,
    const double  t,
    const int     iMajorTimeStep,
    void         *outZeroCrossingSignal
    );

extern const char *rtwMatFileLoaderCollectionCreateInstance(
    int    errorXMLMode,
    void **outMatFileLoaderCollection
    );

extern const char *rtwMatFileLoaderCollectionDestroyInstance(
    int   errorXMLMode,
    void *matFileLoaderCollection
    );

extern const char *rtwMatFileLoaderCollectionAddElement(
    int                   errorXMLMode,
    void                 *matFileLoaderCollection,
    const char           *fileName,
    const char           *varPath,
    const int             extrapolationBeforeFirstDataPointInt,
    const int             interpolationWithinTimeRangeInt,
    const int             extrapolationAfterLastDataPointInt,
    const int             iZeroCrossingSwitch,
    const unsigned char  *ground,
    const char           *className,
    const int             storageDTypeId,
    const int             nDims,
    const int            *dims,
    const int             complexity,
    const int             isFixedPoint,
    const int             dataTypeMode,
    const int             isSigned,
    const int             wordLength,
    const double          slopeAdjustmentFactor,
    const int             fixedExponent,
    const double          bias,
    const int             enumNStrings,
    const char          **enumStrings,
    const int            *enumValues,
    int                   fxpDiagnosticOverflow,
    const char           *fxpBlockPath
    );

extern const char *rtwMatFileLoaderCollectionGetOutput(
    int            errorXMLMode,
    void          *matFileLoaderCollection,
    const size_t   elementId,
    const double   t,
    const int      iMajorTimeStep,
    void         **outOutputValue
    );

extern const char *rtwMATFileLoaderCollectionGetZeroCrossingSignal(
    void         *pMatFileLoaderCollection,
    const size_t  elementId,
    const double  t,
    const int     iMajorTimeStep,
    void         *outZeroCrossingSignal
    );

extern const char *rtwMATFileLoaderCheckMATFileConsistency(
    void           *pMATFileLoader,
    const char     *MATFileName,
    int             errorXMLMode,
    const char     *expDTypeName,
    const int       expNDims,
    const int      *expDims,
    const int       expComplexity,
    const int       isFixedPoint,
    const int       dataTypeMode,
    const int       isSigned,
    const int       wordLength,
    const double    slopeAdjustmentFactor,
    const int       fixedExponent,
    const double    bias,
    const int       expEnumNStrings,
    const char    **expEnumStrings,
    const int      *expEnumValues);

#ifdef __cplusplus
}
#endif

#endif /* sl_fileio_rtw_h */

/* LocalWords:  RSim
 */
