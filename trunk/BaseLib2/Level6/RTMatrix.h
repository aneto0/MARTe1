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

/** 
 * @file
 * @brief BaseLib2 matrix support.
 */
#include "System.h"
#include "Streamable.h"


#if !defined(RT_MATRIX_H_)
#define RT_MATRIX_H_

/** a row of a RTMatrix */
template <class T>
class RTMatrixRow{
    /** the actual data */
    T *row;
public:
    /** initialise */
    inline RTMatrixRow(T *row){
        this->row = row;
    }
    /** access the data */
    inline T &operator[](int col)const{
        return row[col];
    }
};


template <class T> class RTMatrixT;
template <class T> class RefMatrixT;
template <class T> class MatrixT;


/** a Matrix with faster access because of lack of checks */
template <class T>
class _pRTMatrix_ {
public:
    /** n of rows */
    uint32 n;

    /** n of columns */
    uint32 m;

    ///
    T *data;

    ///
    T **row;

    /** Allow fast access to the packed element structure */
    inline T *Data()const{
        return data;
    }

    /** The size as number of MatrixType of the matrix
     ! It semms to me that const is not useful here, parameter returned by value! by Anton */
    inline uint32 DataSize()const{
        return n*m;
    }

};


extern "C" {

    /** parts of RTMatrix*/
    bool RTM__vecIsNull_I(const int    *d,const uint32 sz);
    /** parts of RTMatrix*/
    bool RTM__vecIsNull_F(const float  *d,const uint32 sz);
    /** parts of RTMatrix*/
    bool RTM__vecIsNull_D(const double *d,const uint32 sz);

    /** parts of RTMatrix*/
    void RTM__vecCopy_F (float  *d,const float  *s,const uint32 sz);
    /** parts of RTMatrix*/
    void RTM__vecCopy_D (double *d,const double *s,const uint32 sz);
    /** parts of RTMatrix*/
    void RTM__vecCopy_I (int    *d,const int    *s,const uint32 sz);
    /** parts of RTMatrix*/
    void RTM__vecCopy_DI(double *d,const int    *s,const uint32 sz);
    /** parts of RTMatrix*/
    void RTM__vecCopy_DF(double *d,const float  *s,const uint32 sz);
    /** parts of RTMatrix*/
    void RTM__vecCopy_FI(float  *d,const int    *s,const uint32 sz);
    /** parts of RTMatrix*/
    void RTM__vecCopy_FD(float  *d,const double *s,const uint32 sz);
    /** parts of RTMatrix*/
    void RTM__vecCopy_IF(int    *d,const float  *s,const uint32 sz);
    /** parts of RTMatrix*/
    void RTM__vecCopy_ID(int    *d,const double *s,const uint32 sz);

    /** parts of RTMatrix*/
    void RTM__vecSum_F(float *d,const float *s1,const float *s2,uint32 sz);
    /** parts of RTMatrix*/
    void RTM__vecSum_D(double *d,const double *s1,const double *s2,uint32 sz);
    /** parts of RTMatrix*/
    void RTM__vecSum_I(int *d,const int *s1,const int *s2,uint32 sz);

    /** parts of RTMatrix*/
    void RTM__vecDiff_F(float *d,const float *s1,const float *s2,uint32 sz);
    /** parts of RTMatrix*/
    void RTM__vecDiff_D(double *d,const double *s1,const double *s2,uint32 sz);
    /** parts of RTMatrix*/
    void RTM__vecDiff_I(int *d,const int *s1,const int *s2,uint32 sz);

    /** parts of RTMatrix*/
    void RTM__vecComb_I(int *d,uint32 sz,const int *s1,const int w1,const int *s2,const int w2);
    /** parts of RTMatrix*/
    void RTM__vecComb_F(float *d,uint32 sz,const float *s1,const float w1,const float *s2,const float w2);
    /** parts of RTMatrix*/
    void RTM__vecComb_D(double *d,uint32 sz,const double *s1,const double w1,const double *s2,const double w2);

    /** parts of RTMatrix*/
    int    RTM__vecMax_I(const int    *d,const uint32 sz);
    /** parts of RTMatrix*/
    float  RTM__vecMax_F(const float  *d,const uint32 sz);
    /** parts of RTMatrix*/
    double RTM__vecMax_D(const double *d,const uint32 sz);

    /** parts of RTMatrix*/
    int    RTM__vecMin_I(const int    *d,const uint32 sz);
    /** parts of RTMatrix*/
    float  RTM__vecMin_F(const float  *d,const uint32 sz);
    /** parts of RTMatrix*/
    double RTM__vecMin_D(const double *d,const uint32 sz);

    /** parts of RTMatrix*/
    void RTM__vecScale_F(float *d,uint32 sz,const float w);
    /** parts of RTMatrix*/
    void RTM__vecScale_D(double *d,uint32 sz,const double w);
    /** parts of RTMatrix*/
    void RTM__vecScale_I(int *d,uint32 sz,const int w);

    /** parts of RTMatrix*/
    void RTMZerofy_I(RTMatrixT<int>    &A);
    /** parts of RTMatrix*/
    void RTMZerofy_F(RTMatrixT<float>  &A);
    /** parts of RTMatrix*/
    void RTMZerofy_D(RTMatrixT<double> &A);

    /** parts of RTMatrix*/
    void RTMOnefy_I(RTMatrixT<int>    &A);
    /** parts of RTMatrix*/
    void RTMOnefy_F(RTMatrixT<float>  &A);
    /** parts of RTMatrix*/
    void RTMOnefy_D(RTMatrixT<double> &A);

    /** parts of RTMatrix*/
    void RTMEyefy_I(RTMatrixT<int>    &A);
    /** parts of RTMatrix*/
    void RTMEyefy_F(RTMatrixT<float>  &A);
    /** parts of RTMatrix*/
    void RTMEyefy_D(RTMatrixT<double> &A);

    /** parts of RTMatrix*/
    void RTMScale_II(RTMatrixT<int>    &A,const int    factor);
    /** parts of RTMatrix*/
    void RTMScale_FF(RTMatrixT<float>  &A,const float  factor);
    /** parts of RTMatrix*/
    void RTMScale_DD(RTMatrixT<double> &A,const double factor);

    /** parts of RTMatrix*/
    bool RTMIsNull_I(const RTMatrixT<int>    &A);
    /** parts of RTMatrix*/
    bool RTMIsNull_F(const RTMatrixT<float>  &A);
    /** parts of RTMatrix*/
    bool RTMIsNull_D(const RTMatrixT<double> &A);

    /** parts of RTMatrix*/
    int    RTMMax_I(const RTMatrixT<int>    &ma);
    /** parts of RTMatrix*/
    float  RTMMax_F(const RTMatrixT<float>  &ma);
    /** parts of RTMatrix*/
    double RTMMax_D(const RTMatrixT<double> &ma);

    /** parts of RTMatrix*/
    int    RTMMin_I(const RTMatrixT<int>    &ma);
    /** parts of RTMatrix*/
    float  RTMMin_F(const RTMatrixT<float>  &ma);
    /** parts of RTMatrix*/
    double RTMMin_D(const RTMatrixT<double> &ma);

    /** parts of RTMatrix*/
    void RTMCombine_UIIII(RTMatrixT<int>    &rtm,const RTMatrixT<int   > &A,const int    aw,const RTMatrixT<int   > &B,const int    bw);
    /** parts of RTMatrix*/
    void RTMCombine_UFFFF(RTMatrixT<float>  &rtm,const RTMatrixT<float > &A,const float  aw,const RTMatrixT<float > &B,const float  bw);
    /** parts of RTMatrix*/
    void RTMCombine_UDDDD(RTMatrixT<double> &rtm,const RTMatrixT<double> &A,const double aw,const RTMatrixT<double> &B,const double bw);

    /** parts of RTMatrix*/
    void RTMProduct_UIII(RTMatrixT<int>   &rtm,const RTMatrixT<int>   &A,const RTMatrixT<int>   &B);
    /** parts of RTMatrix*/
    void RTMProduct_UFFF(RTMatrixT<float> &rtm,const RTMatrixT<float> &A,const RTMatrixT<float> &B);
    /** parts of RTMatrix*/
    void RTMProduct_UDDD(RTMatrixT<double>&rtm,const RTMatrixT<double>&A,const RTMatrixT<double>&B);

    /** parts of RTMatrix*/
    void RTMVProduct_UIII(RTMatrixT<int> &rtm,int *input,int *output)         ;
    /** parts of RTMatrix*/
    void RTMVProduct_UFFF(RTMatrixT<float> &rtm,float *input,float *output)   ;
    /** parts of RTMatrix*/
    void RTMVProduct_UDDD(RTMatrixT<double> &rtm,double *input,double *output);

    /** parts of RTMatrix*/
    void RTMVProductAcc_UIII(RTMatrixT<int> &rtm,int *input,int *output)         ;
    /** parts of RTMatrix*/
    void RTMVProductAcc_UFFF(RTMatrixT<float> &rtm,float *input,float *output)   ;
    /** parts of RTMatrix*/
    void RTMVProductAcc_UDDD(RTMatrixT<double> &rtm,double *input,double *output);

    /** parts of RTMatrix*/
    void RTMDotProduct_UII(RTMatrixT<int> &rtm,const RTMatrixT<int> &A);
    /** parts of RTMatrix*/
    void RTMDotProduct_UFF(RTMatrixT<float> &rtm,const RTMatrixT<float> &A);
    /** parts of RTMatrix*/
    void RTMDotProduct_UDD(RTMatrixT<double> &rtm,const RTMatrixT<double> &A);

    /** parts of RTMatrix*/
    int    RTMMaxAbsRowSumNorm_I(RTMatrixT<int>    &A);
    /** parts of RTMatrix*/
    float  RTMMaxAbsRowSumNorm_F(RTMatrixT<float>  &A);
    /** parts of RTMatrix*/
    double RTMMaxAbsRowSumNorm_D(RTMatrixT<double> &A);

    /** parts of RTMatrix*/
    int    RTMMaxAbsColSumNorm_I(RTMatrixT<int>    &A);
    /** parts of RTMatrix*/
    float  RTMMaxAbsColSumNorm_F(RTMatrixT<float>  &A);
    /** parts of RTMatrix*/
    double RTMMaxAbsColSumNorm_D(RTMatrixT<double> &A);

    /** parts of RTMatrix*/
    void RTMTranspose_UII(RTMatrixT<int   > &rtm,const RTMatrixT<int   > &A);
    /** parts of RTMatrix*/
    void RTMTranspose_UFF(RTMatrixT<float > &rtm,const RTMatrixT<float > &A);
    /** parts of RTMatrix*/
    void RTMTranspose_UDD(RTMatrixT<double> &rtm,const RTMatrixT<double> &A);

    /** parts of RTMatrix*/
    bool RTMInvert_UFF(RTMatrixT<float > &rtm,RTMatrixT<float > &A);
    /** parts of RTMatrix*/
    bool RTMInvert_UDF(RTMatrixT<double> &rtm,RTMatrixT<float > &A);
    /** parts of RTMatrix*/
    bool RTMInvert_UDD(RTMatrixT<double> &rtm,RTMatrixT<double> &A);

    /** parts of RTMatrix*/
    bool RTMLU_FFFF(RTMatrixT<float > &rtm,RTMatrixT<float > &L,RTMatrixT<float > &U,RTMatrixT<float > &P);
    /** parts of RTMatrix*/
    bool RTMLU_DDDD(RTMatrixT<double> &rtm,RTMatrixT<double> &L,RTMatrixT<double> &U,RTMatrixT<double> &P);

    /** parts of RTMatrix*/
    bool RTMSVD_FFFF(RTMatrixT<float > &rtm,RTMatrixT<float > &U, RTMatrixT<float > &W, RTMatrixT<float > &V);
    /** parts of RTMatrix*/
    bool RTMSVD_DDDD(RTMatrixT<double> &rtm,RTMatrixT<double> &U, RTMatrixT<double> &W, RTMatrixT<double> &V);

    /** parts of RTMatrix*/
    void RTMPrint_I(RTMatrixT<int   > &rtm,StreamInterface &s,int maxrow);
    /** parts of RTMatrix*/
    void RTMPrint_F(RTMatrixT<float > &rtm,StreamInterface &s,int maxrow);
    /** parts of RTMatrix*/
    void RTMPrint_D(RTMatrixT<double> &rtm,StreamInterface &s,int maxrow);

    /** parts of RTMatrix*/
    bool RTMAllocate_I(RTMatrixT<int   > &rtm,uint32 nRows,uint32 nColumns);
    /** parts of RTMatrix*/
    bool RTMAllocate_F(RTMatrixT<float > &rtm,uint32 nRows,uint32 nColumns);
    /** parts of RTMatrix*/
    bool RTMAllocate_D(RTMatrixT<double> &rtm,uint32 nRows,uint32 nColumns);

    /** parts of RTMatrix*/
    void REFMAllocate_I(RefMatrixT<int   > &refm);
    /** parts of RTMatrix*/
    void REFMAllocate_F(RefMatrixT<float > &refm);
    /** parts of RTMatrix*/
    void REFMAllocate_D(RefMatrixT<double> &refm);

    /** parts of RTMatrix*/
    void REFMDeAllocate_I(RefMatrixT<int   > &refm);
    /** parts of RTMatrix*/
    void REFMDeAllocate_F(RefMatrixT<float > &refm);
    /** parts of RTMatrix*/
    void REFMDeAllocate_D(RefMatrixT<double> &refm);

    /** parts of RTMatrix*/
    void MAReAllocate_I(MatrixT<int   > &ma,uint32 nRows,uint32 nColumns);
    /** parts of RTMatrix*/
    void MAReAllocate_F(MatrixT<float > &ma,uint32 nRows,uint32 nColumns);
    /** parts of RTMatrix*/
    void MAReAllocate_D(MatrixT<double> &ma,uint32 nRows,uint32 nColumns);

}


static inline bool RTM__vecIsNull(const int*    d,const uint32 sz){return RTM__vecIsNull_I(d,sz);}
static inline bool RTM__vecIsNull(const float*  d,const uint32 sz){return RTM__vecIsNull_F(d,sz);}
static inline bool RTM__vecIsNull(const double* d,const uint32 sz){return RTM__vecIsNull_D(d,sz);}

static inline void RTM__vecCopy(float  *d,const float  *s,const uint32 sz)                                            { RTM__vecCopy_F (d,s,sz); }
static inline void RTM__vecCopy(double *d,const double *s,const uint32 sz)                                            { RTM__vecCopy_D (d,s,sz); }
static inline void RTM__vecCopy(int    *d,const int    *s,const uint32 sz)                                            { RTM__vecCopy_I (d,s,sz); }
static inline void RTM__vecCopy(double *d,const float  *s,const uint32 sz)                                            { RTM__vecCopy_DF(d,s,sz); }
static inline void RTM__vecCopy(double *d,const int    *s,const uint32 sz)                                            { RTM__vecCopy_DI(d,s,sz); }
static inline void RTM__vecCopy(float  *d,const double *s,const uint32 sz)                                            { RTM__vecCopy_FD(d,s,sz); }
static inline void RTM__vecCopy(float  *d,const int    *s,const uint32 sz)                                            { RTM__vecCopy_FI(d,s,sz); }
static inline void RTM__vecCopy(int    *d,const float  *s,const uint32 sz)                                            { RTM__vecCopy_IF(d,s,sz); }
static inline void RTM__vecCopy(int    *d,const double *s,const uint32 sz)                                            { RTM__vecCopy_ID(d,s,sz); }

static inline void RTM__vecSum(int    *d,const int    *s1,const int    *s2,uint32 sz)                                 { RTM__vecSum_I(d,s1,s2,sz); }
static inline void RTM__vecSum(float  *d,const float  *s1,const float  *s2,uint32 sz)                                 { RTM__vecSum_F(d,s1,s2,sz); }
static inline void RTM__vecSum(double *d,const double *s1,const double *s2,uint32 sz)                                 { RTM__vecSum_D(d,s1,s2,sz); }

static inline void RTM__vecDiff(int    *d,const int    *s1,const int    *s2,uint32 sz)                                { RTM__vecDiff_I(d,s1,s2,sz); }
static inline void RTM__vecDiff(float  *d,const float  *s1,const float  *s2,uint32 sz)                                { RTM__vecDiff_F(d,s1,s2,sz); }
static inline void RTM__vecDiff(double *d,const double *s1,const double *s2,uint32 sz)                                { RTM__vecDiff_D(d,s1,s2,sz); }

// not likely to have differet parameters sequence for these functions and RTMCombine_X functions...
static inline void RTM__vecComb(int    *d,uint32 sz,const int    *s1,const int    w1,const int    *s2,const int    w2){ RTM__vecComb_I(d,sz,s1,w1,s2,w2); }
static inline void RTM__vecComb(float  *d,uint32 sz,const float  *s1,const float  w1,const float  *s2,const float  w2){ RTM__vecComb_F(d,sz,s1,w1,s2,w2); }
static inline void RTM__vecComb(double *d,uint32 sz,const double *s1,const double w1,const double *s2,const double w2){ RTM__vecComb_D(d,sz,s1,w1,s2,w2); }

static inline int    RTM__vecMax(const int    *d,const uint32 sz){return RTM__vecMax_I(d,sz);}
static inline float  RTM__vecMax(const float  *d,const uint32 sz){return RTM__vecMax_F(d,sz);}
static inline double RTM__vecMax(const double *d,const uint32 sz){return RTM__vecMax_D(d,sz);}

static inline int    RTM__vecMin(const int    *d,const uint32 sz){return RTM__vecMin_I(d,sz);}
static inline float  RTM__vecMin(const float  *d,const uint32 sz){return RTM__vecMin_F(d,sz);}
static inline double RTM__vecMin(const double *d,const uint32 sz){return RTM__vecMin_D(d,sz);}

static inline void RTM__vecScale(float *d,const float w,uint32 sz)                                                   { RTM__vecScale_F(d,sz,w); }
static inline void RTM__vecScale(double *d,const double w,uint32 sz)                                                 { RTM__vecScale_D(d,sz,w); }
static inline void RTM__vecScale(int *d,const int w,uint32 sz)                                                       { RTM__vecScale_I(d,sz,w); }

static inline void RTMZerofy(RTMatrixT<int>    &A){RTMZerofy_I(A);}
static inline void RTMZerofy(RTMatrixT<float>  &A){RTMZerofy_F(A);}
static inline void RTMZerofy(RTMatrixT<double> &A){RTMZerofy_D(A);}

static inline void RTMOnefy(RTMatrixT<int>    &A){RTMOnefy_I(A);}
static inline void RTMOnefy(RTMatrixT<float>  &A){RTMOnefy_F(A);}
static inline void RTMOnefy(RTMatrixT<double> &A){RTMOnefy_D(A);}

static inline void RTMEyefy(RTMatrixT<int>    &A){RTMEyefy_I(A);}
static inline void RTMEyefy(RTMatrixT<float>  &A){RTMEyefy_F(A);}
static inline void RTMEyefy(RTMatrixT<double> &A){RTMEyefy_D(A);}

static inline void RTMScale(RTMatrixT<int>    &A,const int    factor){RTMScale_II(A,factor);}
static inline void RTMScale(RTMatrixT<float>  &A,const float  factor){RTMScale_FF(A,factor);}
static inline void RTMScale(RTMatrixT<double> &A,const double factor){RTMScale_DD(A,factor);}

static inline bool RTMIsNull(const RTMatrixT<int>    &A){return RTMIsNull_I(A);}
static inline bool RTMIsNull(const RTMatrixT<float>  &A){return RTMIsNull_F(A);}
static inline bool RTMIsNull(const RTMatrixT<double> &A){return RTMIsNull_D(A);}

static inline int    RTMMax(const RTMatrixT<int>    &ma){return RTMMax_I(ma);}
static inline float  RTMMax(const RTMatrixT<float>  &ma){return RTMMax_F(ma);}
static inline double RTMMax(const RTMatrixT<double> &ma){return RTMMax_D(ma);}

static inline int    RTMMin(const RTMatrixT<int>    &ma){return RTMMin_I(ma);}
static inline float  RTMMin(const RTMatrixT<float>  &ma){return RTMMin_F(ma);}
static inline double RTMMin(const RTMatrixT<double> &ma){return RTMMin_D(ma);}

static inline void RTMProduct_U(RTMatrixT<int>&dest,    const RTMatrixT<int> &A   ,const RTMatrixT<int> &B){    RTMProduct_UIII(dest,A,B);}
static inline void RTMProduct_U(RTMatrixT<float>&dest,  const RTMatrixT<float> &A ,const RTMatrixT<float> &B){  RTMProduct_UFFF(dest,A,B);}
static inline void RTMProduct_U(RTMatrixT<double>&dest, const RTMatrixT<double> &A,const RTMatrixT<double> &B){ RTMProduct_UDDD(dest,A,B);}

static inline void RTMVProduct_U(RTMatrixT<int>&dest,    int    *input,int    *output){ RTMVProduct_UIII(dest,input,output);}
static inline void RTMVProduct_U(RTMatrixT<float>&dest,  float  *input,float  *output){ RTMVProduct_UFFF(dest,input,output);}
static inline void RTMVProduct_U(RTMatrixT<double>&dest, double *input,double *output){ RTMVProduct_UDDD(dest,input,output);}

static inline void RTMVProductAcc_U(RTMatrixT<int>   &dest, int    *input,int    *output){ RTMVProductAcc_UIII(dest,input,output);}
static inline void RTMVProductAcc_U(RTMatrixT<float> &dest, float  *input,float  *output){ RTMVProductAcc_UFFF(dest,input,output);}
static inline void RTMVProductAcc_U(RTMatrixT<double>&dest, double *input,double *output){ RTMVProductAcc_UDDD(dest,input,output);}

static inline void RTMDotProduct_U(RTMatrixT<int>   &rtm,const RTMatrixT<int>   &A){ RTMDotProduct_UII(rtm,A); }
static inline void RTMDotProduct_U(RTMatrixT<float> &rtm,const RTMatrixT<float> &A){ RTMDotProduct_UFF(rtm,A); }
static inline void RTMDotProduct_U(RTMatrixT<double>&rtm,const RTMatrixT<double>&A){ RTMDotProduct_UDD(rtm,A); }

static inline void RTMSum_U(_pRTMatrix_<int>   &rtm,const _pRTMatrix_<int>   &A,const _pRTMatrix_<int>   &B){ RTM__vecSum_I(rtm.data,A.data,B.data,rtm.DataSize());}
static inline void RTMSum_U(_pRTMatrix_<float> &rtm,const _pRTMatrix_<float> &A,const _pRTMatrix_<float> &B){ RTM__vecSum_F(rtm.data,A.data,B.data,rtm.DataSize());}
static inline void RTMSum_U(_pRTMatrix_<double>&rtm,const _pRTMatrix_<double>&A,const _pRTMatrix_<double>&B){ RTM__vecSum_D(rtm.data,A.data,B.data,rtm.DataSize());}

static inline void RTMDiff_U(_pRTMatrix_<int>   &rtm,const _pRTMatrix_<int   >&A,const _pRTMatrix_<int>   &B){ RTM__vecDiff_I(rtm.data,A.data,B.data,rtm.DataSize());}
static inline void RTMDiff_U(_pRTMatrix_<float> &rtm,const _pRTMatrix_<float >&A,const _pRTMatrix_<float> &B){ RTM__vecDiff_F(rtm.data,A.data,B.data,rtm.DataSize());}
static inline void RTMDiff_U(_pRTMatrix_<double>&rtm,const _pRTMatrix_<double>&A,const _pRTMatrix_<double>&B){ RTM__vecDiff_D(rtm.data,A.data,B.data,rtm.DataSize());}

static inline void RTMCombine_U(_pRTMatrix_<int>   &rtm,const _pRTMatrix_<int   >&A,const int    aw,const _pRTMatrix_<int   >&B,const int    bw){RTM__vecComb_I(rtm.data,rtm.DataSize(),A.data,aw,B.data,bw);}
static inline void RTMCombine_U(_pRTMatrix_<float> &rtm,const _pRTMatrix_<float >&A,const float  aw,const _pRTMatrix_<float >&B,const float  bw){RTM__vecComb_F(rtm.data,rtm.DataSize(),A.data,aw,B.data,bw);}
static inline void RTMCombine_U(_pRTMatrix_<double>&rtm,const _pRTMatrix_<double>&A,const double aw,const _pRTMatrix_<double>&B,const double bw){RTM__vecComb_D(rtm.data,rtm.DataSize(),A.data,aw,B.data,bw);}

static inline int    RTMMaxAbsRowSumNorm(RTMatrixT<int>    &A){return RTMMaxAbsRowSumNorm_I(A);}
static inline float  RTMMaxAbsRowSumNorm(RTMatrixT<float>  &A){return RTMMaxAbsRowSumNorm_F(A);}
static inline double RTMMaxAbsRowSumNorm(RTMatrixT<double> &A){return RTMMaxAbsRowSumNorm_D(A);}

static inline int    RTMMaxAbsColSumNorm(RTMatrixT<int>    &A){return RTMMaxAbsColSumNorm_I(A);}
static inline float  RTMMaxAbsColSumNorm(RTMatrixT<float>  &A){return RTMMaxAbsColSumNorm_F(A);}
static inline double RTMMaxAbsColSumNorm(RTMatrixT<double> &A){return RTMMaxAbsColSumNorm_D(A);}

static inline void RTMTranspose_U(RTMatrixT<int   >&rtm,const RTMatrixT<int>   &A){ RTMTranspose_UII(rtm,A); }
static inline void RTMTranspose_U(RTMatrixT<float >&rtm,const RTMatrixT<float> &A){ RTMTranspose_UFF(rtm,A); }
static inline void RTMTranspose_U(RTMatrixT<double>&rtm,const RTMatrixT<double>&A){ RTMTranspose_UDD(rtm,A); }

static inline bool RTMInvert_U(RTMatrixT<int   >&rtm,RTMatrixT<int   >&A){ return False; }
static inline bool RTMInvert_U(RTMatrixT<float >&rtm,RTMatrixT<float >&A){ return RTMInvert_UFF(rtm,A); }
static inline bool RTMInvert_U(RTMatrixT<double>&rtm,RTMatrixT<float >&A){ return RTMInvert_UDF(rtm,A); }
static inline bool RTMInvert_U(RTMatrixT<double>&rtm,RTMatrixT<double>&A){ return RTMInvert_UDD(rtm,A); }

static inline bool RTMLU(RTMatrixT<int   > &rtm,RTMatrixT<int   > &L,RTMatrixT<int   > &U,RTMatrixT<int   > &P){ return False; }
static inline bool RTMLU(RTMatrixT<float > &rtm,RTMatrixT<float > &L,RTMatrixT<float > &U,RTMatrixT<float > &P){ return RTMLU_FFFF(rtm,L,U,P); }
static inline bool RTMLU(RTMatrixT<double> &rtm,RTMatrixT<double> &L,RTMatrixT<double> &U,RTMatrixT<double> &P){ return RTMLU_DDDD(rtm,L,U,P); }

static inline bool RTMSVD(RTMatrixT<int   > &rtm,RTMatrixT<int   > &U, RTMatrixT<int   > &W, RTMatrixT<int   > &V){ return False; }
static inline bool RTMSVD(RTMatrixT<float > &rtm,RTMatrixT<float > &U, RTMatrixT<float > &W, RTMatrixT<float > &V){ return RTMSVD_FFFF(rtm,U, W, V); }
static inline bool RTMSVD(RTMatrixT<double> &rtm,RTMatrixT<double> &U, RTMatrixT<double> &W, RTMatrixT<double> &V){ return RTMSVD_DDDD(rtm,U, W, V); }

static inline void RTMPrint(RTMatrixT<int   > &rtm,StreamInterface &s,int maxrow){ RTMPrint_I(rtm,s,maxrow); }
static inline void RTMPrint(RTMatrixT<float > &rtm,StreamInterface &s,int maxrow){ RTMPrint_F(rtm,s,maxrow); }
static inline void RTMPrint(RTMatrixT<double> &rtm,StreamInterface &s,int maxrow){ RTMPrint_D(rtm,s,maxrow); }

static inline bool RTMAllocate(RTMatrixT<int   > &rtm,uint32 nRows,uint32 nColumns){ return RTMAllocate_I(rtm,nRows,nColumns); }
static inline bool RTMAllocate(RTMatrixT<float > &rtm,uint32 nRows,uint32 nColumns){ return RTMAllocate_F(rtm,nRows,nColumns); }
static inline bool RTMAllocate(RTMatrixT<double> &rtm,uint32 nRows,uint32 nColumns){ return RTMAllocate_D(rtm,nRows,nColumns); }

static inline void REFMAllocate(RefMatrixT<int   > &refm){ REFMAllocate_I(refm); }
static inline void REFMAllocate(RefMatrixT<float > &refm){ REFMAllocate_F(refm); }
static inline void REFMAllocate(RefMatrixT<double> &refm){ REFMAllocate_D(refm); }

static inline void REFMDeAllocate(RefMatrixT<int   > &refm){ REFMDeAllocate_I(refm); }
static inline void REFMDeAllocate(RefMatrixT<float > &refm){ REFMDeAllocate_F(refm); }
static inline void REFMDeAllocate(RefMatrixT<double> &refm){ REFMDeAllocate_D(refm); }

static inline void MAReAllocate(MatrixT<int   > &ma,uint32 nRows,uint32 nColumns){ MAReAllocate_I(ma,nRows,nColumns); }
static inline void MAReAllocate(MatrixT<float > &ma,uint32 nRows,uint32 nColumns){ MAReAllocate_F(ma,nRows,nColumns); }
static inline void MAReAllocate(MatrixT<double> &ma,uint32 nRows,uint32 nColumns){ MAReAllocate_D(ma,nRows,nColumns); }



/** copies a matrix */
template <class T1,class T2>
inline bool RTMCopy(RTMatrixT<T1> &dest, const RTMatrixT<T2> &src){
    if (dest.m != src.m) return False;
    if (dest.n != src.n) return False;

    uint32 sz = src.DataSize();
    RTM__vecCopy(dest.data,src.data,sz);

    return True;
}

/** loads a packed array of data as floats */
template <class T1,class T2>
inline bool RTMLoad(RTMatrixT<T1> &dest,const T2 *src, const uint32 nRows,const uint32 nColumns){
    if (dest.m != nColumns) return False;
    if (dest.n != nRows) return False;

    uint32 sz = dest.DataSize();
    RTM__vecCopy(dest.data,src,sz);

    return True;
}

/** saves the matrix into a packed array of data */
template <class T1,class T2>
inline bool RTMSave(RTMatrixT<T1> &src,T2 *dest, const uint32 nRows,const uint32 nColumns){
    if (src.m != nColumns) return False;
    if (src.n != nRows) return False;

    uint32 sz = src.DataSize();
    RTM__vecCopy(dest,src.data,sz);
    return True;
}

/** The actual RTMatrix  */
template <class T>
class RTMatrixT:public _pRTMatrix_<T> {

private:
   /**
    * Copy or initialize an RTMatrix.
    * @param A The matrix to copy.
    */
    void Copy(const RTMatrixT<T> &A){
        if (A.data!=NULL){
	    Allocate(A.NRows(),A.NColumns());
	    RTMLoad(*this,A.data,A.NRows(),A.NColumns());
	}
	else{
            Initialize();
	}
    }
            
public:
    ///
    bool Allocate(const uint32 nRows,const uint32 nColumns){
        return RTMAllocate(*this,nRows,nColumns);
    }

    ///
    void DeAllocate(){
        if (this->data != NULL) free((void *&)this->data);
        if (this->row  != NULL) free((void *&)this->row);
        Initialize();
    }

    ///
    void Initialize(){
        this->data = NULL;
        this->row = NULL;
        this->n = 0;
        this->m = 0;
    }

    ///
    RTMatrixT(){
        Initialize();
    }

    ///
    inline void SwapRows(uint32 row_i, uint32 row_j){
        T temp;
        T *Rowi = this->row[row_i];
        T *Rowj = this->row[row_j];
        for(int i=0;i<this->m;i++){
            temp  = Rowj[i];
            Rowj[i] = Rowi[i];
            Rowi[i] = temp;
        }
    }

public:
    ///
    inline RTMatrixT(const uint32 nRows,const uint32 nColumns){
        Allocate(nRows,nColumns);
    }

    ///
    inline RTMatrixT(const uint32 nRows,const uint32 nColumns,const T *source){
        Allocate(nRows,nColumns);
        RTMLoad(*this,source,nRows,nColumns);
    }

    ///
    inline RTMatrixT(const RTMatrixT<T> &A){
	Copy(A);
    }

    /**  Constructor for allocating a zeros,ones,eye matrix todo */
    inline RTMatrixT(const uint32 nRows,const uint32 nColumns,const char* options){

    }

    ///
    inline ~RTMatrixT(){
        DeAllocate();
    }

    /** Set elements of the matrix to zero. */
    inline void Zerofy(){
        RTMZerofy(*this);
    }

    /** Set elements of the matrix to one. */
    inline void Onefy(){
        RTMOnefy(*this);
    }

    /** Set the diagonal entries of the matrix to one, the others to zero. */
    inline void Eyefy(){
        RTMEyefy(*this);
    }

    /** scale all the values by x*/
    inline void Scale(const T &x){
        RTMScale(*this,x);
    }

    /** n of rows */
    inline uint32 NRows()const{
        return this->n;
    }

    /** n of columns */
    inline uint32 NColumns()const{
        return this->m;
    }

    /** Set elements of the matrix to zero. */
    inline void Clear(){
        T *p = this->data;
        T *pEnd = p + (this->n*this->m);
        while(p<pEnd){
            *p++ = (T)0;
        }
    }

    /** Allow fast access to elements */
    inline const RTMatrixRow<T> operator[](int rowNo){
        return RTMatrixRow<T>(this->row[rowNo]);
    }

    /** Returns True if the matrix is the null one. */
    inline bool IsNull(){
        return RTMIsNull(*this);
    }

    /** loads the product of A and B */
    inline bool Product(const RTMatrixT &A,const RTMatrixT &B){
        if (A.m != B.n) return False;
        if (this->n != A.n) return False;
        if (this->m != B.m) return False;
        RTMProduct_U(*this,A,B);
        return True;
    }

    /** this is an element by element product */
    inline bool DotProduct(const RTMatrixT<T> &A){
        if (A.m != this->m) return False;
        if (A.n != this->n) return False;
        RTMDotProduct_U(*this,A);
        return True;
    }

    /** loads the sum of A and B */
    inline bool Sum(const RTMatrixT<T> &A,const RTMatrixT<T> &B){
        if (A.m != this->m) return False;
        if (A.n != this->n) return False;
        if (B.m != this->m) return False;
        if (B.n != this->n) return False;
        RTMSum_U(*this,A,B);
        return True;
    }

    /** loads the difference of A and B */
    inline bool Diff(const RTMatrixT<T> &A,const RTMatrixT<T> &B){
        if (A.m != this->m) return False;
        if (A.n != this->n) return False;
        if (B.m != this->m) return False;
        if (B.n != this->n) return False;
        RTMDiff_U(*this,A,B);
        return True;
    }

    /** loads the linear combination of A and B
     WARNING: Uncoherent notation with the parameters in RTM__vecComb!!!
     to restabilish eveness maybe is better to change the definition in that functions set. */
    inline bool Combine(const RTMatrixT<T> &A,const T aw,const RTMatrixT<T> &B,const T bw){
        if (A.m != this->m) return False;
        if (A.n != this->n) return False;
        if (B.m != this->m) return False;
        if (B.n != this->n) return False;
        RTMCombine_U(*this,A,aw,B,bw);
        return True;
    }

    /** */
    inline T MaxAbsRowSumNorm(){
        return RTMMaxAbsRowSumNorm(*this);
    }

    /** */
    inline T MaxAbsColSumNorm(){
        return RTMMaxAbsColSumNorm(*this);
    }

    /** loads the transposed of A */
    inline bool Transpose(const RTMatrixT<T> &A){
        if (A.m != this->n) return False;
        if (A.n != this->m) return False;
        RTMTranspose_U(*this,A);
        return True;
    }

    /** inverts the matrix A */
    inline bool Invert(RTMatrixT<T> &A){
        if (this->n != this->m) return False;
        if (A.m != this->m) return False;
        if (A.n != this->n) return False;
        return RTMInvert_U(*this,A);
    }

    inline RTMatrixT &operator=(const RTMatrixT<T> &A){
	Copy(A);
    }

    /** LUP Decomposition */
    bool LU(RTMatrixT<T> &L,RTMatrixT<T> &U,RTMatrixT<T> &P){
        return RTMLU(*this,L,U,P);
    }

    /** SVD Decomposition */
    bool SVD(RTMatrixT<T> &U,RTMatrixT<T> &W,RTMatrixT<T> &V){
        return RTMSVD(*this,U, W, V);
    }

    /** just dumps on stream */
    void Print(StreamInterface &s,int maxrow=10){
        RTMPrint(*this,s,maxrow);
    }

};

/** */
#define RTMatrixI    RTMatrixT<int>

/** */
#define RTMatrixF    RTMatrixT<float>

/** */
#define RTMatrixD    RTMatrixT<double>

/** */
#define RTMLOAD_MATRIX(matrix,data) RTMLoad(matrix,data[0],sizeof(data)/sizeof(data[0]),sizeof(data[0])/sizeof(data[0][0]));

/** */
#define RTMLOAD_VECTOR(matrix,data) RTMLoad(matrix,data,1,sizeof(data)/sizeof(data[0]));


#endif



