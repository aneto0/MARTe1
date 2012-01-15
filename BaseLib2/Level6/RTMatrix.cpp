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


#include "RTMatrix.h"
#include "RefMatrix.h"
#include "Matrix.h"
#include "System.h"

template <class T>
    inline bool RTM__vecIsNull_T(const T* d,const uint32 sz){
        const T* df=d+sz;
        while (d<df-16){
            if (d[ 0] != 0) return False;
            if (d[ 1] != 0) return False;
            if (d[ 2] != 0) return False;
            if (d[ 3] != 0) return False;
            if (d[ 4] != 0) return False;
            if (d[ 5] != 0) return False;
            if (d[ 6] != 0) return False;
            if (d[ 7] != 0) return False;
            if (d[ 8] != 0) return False;
            if (d[ 9] != 0) return False;
            if (d[10] != 0) return False;
            if (d[11] != 0) return False;
            if (d[12] != 0) return False;
            if (d[13] != 0) return False;
            if (d[14] != 0) return False;
            if (d[15] != 0) return False;
            d += 16;
        }
        while (d<df){
//            if (*d != 0) return False;
//            d++;
            if (*d++ != 0) return False;
        }
        return True;
    }

template <class T1,class T2>
    inline void RTM__vecCopy_T(T1 *d,const T2 *s,const uint32 sz){
        T1 *df = d + sz;
        while(d<(df-16)){
            d[ 0] = (T1)s[ 0];
            d[ 1] = (T1)s[ 1];
            d[ 2] = (T1)s[ 2];
            d[ 3] = (T1)s[ 3];
            d[ 4] = (T1)s[ 4];
            d[ 5] = (T1)s[ 5];
            d[ 6] = (T1)s[ 6];
            d[ 7] = (T1)s[ 7];
            d[ 8] = (T1)s[ 8];
            d[ 9] = (T1)s[ 9];
            d[10] = (T1)s[10];
            d[11] = (T1)s[11];
            d[12] = (T1)s[12];
            d[13] = (T1)s[13];
            d[14] = (T1)s[14];
            d[15] = (T1)s[15];
            d += 16;
            s += 16;
        }
        while(d<df){
	    d[0] = (T1)s[0];
	    d++;
	    s++;
	}
    }

template <class T>
    inline void RTM__vecSum_T(T *d,const T *s1,const T *s2,const uint32 sz){
        uint32 szb = sz >> 4;
        T *df = d + sz;
        for(int i = 0;i < szb;i++) {
            d[ 0] = s1[ 0] + s2[ 0];
            d[ 1] = s1[ 1] + s2[ 1];
            d[ 2] = s1[ 2] + s2[ 2];
            d[ 3] = s1[ 3] + s2[ 3];
            d[ 4] = s1[ 4] + s2[ 4];
            d[ 5] = s1[ 5] + s2[ 5];
            d[ 6] = s1[ 6] + s2[ 6];
            d[ 7] = s1[ 7] + s2[ 7];
            d[ 8] = s1[ 8] + s2[ 8];
            d[ 9] = s1[ 9] + s2[ 9];
            d[10] = s1[10] + s2[10];
            d[11] = s1[11] + s2[11];
            d[12] = s1[12] + s2[12];
            d[13] = s1[13] + s2[13];
            d[14] = s1[14] + s2[14];
            d[15] = s1[15] + s2[15];
            d += 16;
            s1 += 16;
            s2 += 16;
        }
        while(d<df) *d++ = *s1++ + *s2++;
    }

template <class T>
    inline void RTM__vecSum_T_soppelsa(T *d,const T *s1,const T *s2,const uint32 sz){
        T *df = d + sz;
        while (d<df-16){
            d[ 0] = s1[ 0] + s2[ 0];
            d[ 1] = s1[ 1] + s2[ 1];
            d[ 2] = s1[ 2] + s2[ 2];
            d[ 3] = s1[ 3] + s2[ 3];
            d[ 4] = s1[ 4] + s2[ 4];
            d[ 5] = s1[ 5] + s2[ 5];
            d[ 6] = s1[ 6] + s2[ 6];
            d[ 7] = s1[ 7] + s2[ 7];
            d[ 8] = s1[ 8] + s2[ 8];
            d[ 9] = s1[ 9] + s2[ 9];
            d[10] = s1[10] + s2[10];
            d[11] = s1[11] + s2[11];
            d[12] = s1[12] + s2[12];
            d[13] = s1[13] + s2[13];
            d[14] = s1[14] + s2[14];
            d[15] = s1[15] + s2[15];
            d += 16;
            s1 += 16;
            s2 += 16;
        }
        while(d<df) *d++ = *s1++ + *s2++;
    }

template <class T>
    inline void RTM__vecDiff_T(T *d,const T *s1,const T *s2,const uint32 sz){
        uint32 szb = sz >> 4;
        T *df = d + sz;
        for(int i = 0;i < szb;i++) {
            d[ 0] = s1[ 0] - s2[ 0];
            d[ 1] = s1[ 1] - s2[ 1];
            d[ 2] = s1[ 2] - s2[ 2];
            d[ 3] = s1[ 3] - s2[ 3];
            d[ 4] = s1[ 4] - s2[ 4];
            d[ 5] = s1[ 5] - s2[ 5];
            d[ 6] = s1[ 6] - s2[ 6];
            d[ 7] = s1[ 7] - s2[ 7];
            d[ 8] = s1[ 8] - s2[ 8];
            d[ 9] = s1[ 9] - s2[ 9];
            d[10] = s1[10] - s2[10];
            d[11] = s1[11] - s2[11];
            d[12] = s1[12] - s2[12];
            d[13] = s1[13] - s2[13];
            d[14] = s1[14] - s2[14];
            d[15] = s1[15] - s2[15];
            d += 16;
            s1 += 16;
            s2 += 16;
        }
        while(d<df) *d++ = *s1++ - *s2++;
    }

// N.B.! Previously below there there was a combination vith the minus sign. I changed it with a +.
// By Anton
template <class T>
    inline void RTM__vecComb_T(T *d,const uint32 sz,const T *s1,const T w1,const T *s2,const T w2){
        uint32 szb = sz >> 4;
        T *df = d + sz;
        for(int i = 0;i < szb;i++) {
            d[ 0] = s1[ 0] * w1 + s2[ 0] * w2;
            d[ 1] = s1[ 1] * w1 + s2[ 1] * w2;
            d[ 2] = s1[ 2] * w1 + s2[ 2] * w2;
            d[ 3] = s1[ 3] * w1 + s2[ 3] * w2;
            d[ 4] = s1[ 4] * w1 + s2[ 4] * w2;
            d[ 5] = s1[ 5] * w1 + s2[ 5] * w2;
            d[ 6] = s1[ 6] * w1 + s2[ 6] * w2;
            d[ 7] = s1[ 7] * w1 + s2[ 7] * w2;
            d[ 8] = s1[ 8] * w1 + s2[ 8] * w2;
            d[ 9] = s1[ 9] * w1 + s2[ 9] * w2;
            d[10] = s1[10] * w1 + s2[10] * w2;
            d[11] = s1[11] * w1 + s2[11] * w2;
            d[12] = s1[12] * w1 + s2[12] * w2;
            d[13] = s1[13] * w1 + s2[13] * w2;
            d[14] = s1[14] * w1 + s2[14] * w2;
            d[15] = s1[15] * w1 + s2[15] * w2;
            d += 16;
            s1 += 16;
            s2 += 16;
        }
        while(d<df) *d++ = *s1++ *w1 + *s2++ *w2;
    }

// Ugly that is sz not immediatly following d.
// I will change
template <class T>
    inline void RTM__vecScale_T(T *d,const uint32 sz,const T w){
        uint32 szb = sz >> 4;
        T *df = d + sz;
        for(int i = 0;i < szb;i++) {
            d[ 0] *= w;
            d[ 1] *= w;
            d[ 2] *= w;
            d[ 3] *= w;
            d[ 4] *= w;
            d[ 5] *= w;
            d[ 6] *= w;
            d[ 7] *= w;
            d[ 8] *= w;
            d[ 9] *= w;
            d[10] *= w;
            d[11] *= w;
            d[12] *= w;
            d[13] *= w;
            d[14] *= w;
            d[15] *= w;
            d += 16;
        }
        while(d<df) *d++ *= w;
    }


template <class T>
    inline T RTM__vecMax_T(T *d,const uint32 sz){
        T max = d[0];
        uint32 szb = sz >> 4;
        T *df = d + sz;
        for(int i = 0;i < szb;i++) {
            if (d[ 0] > max) max = d[ 0];
            if (d[ 1] > max) max = d[ 1];
            if (d[ 2] > max) max = d[ 2];
            if (d[ 3] > max) max = d[ 3];
            if (d[ 4] > max) max = d[ 4];
            if (d[ 5] > max) max = d[ 5];
            if (d[ 6] > max) max = d[ 6];
            if (d[ 7] > max) max = d[ 7];
            if (d[ 8] > max) max = d[ 8];
            if (d[ 9] > max) max = d[ 9];
            if (d[10] > max) max = d[10];
            if (d[11] > max) max = d[11];
            if (d[12] > max) max = d[12];
            if (d[13] > max) max = d[13];
            if (d[14] > max) max = d[14];
            if (d[15] > max) max = d[15];
            d += 16;
        }
        while(d<df) {
//            if (*d > max) max = *d++;
            if (*d > max) max = *d;
            d++;
        }
        return max;
    }

template <class T>
    inline T RTM__vecMin_T(T *d,const uint32 sz){
        T min = d[0];
        uint32 szb = sz >> 4;
        T *df = d + sz;
        for(int i = 0;i < szb;i++) {
            if (d[ 0] < min) min = d[ 0];
            if (d[ 1] < min) min = d[ 1];
            if (d[ 2] < min) min = d[ 2];
            if (d[ 3] < min) min = d[ 3];
            if (d[ 4] < min) min = d[ 4];
            if (d[ 5] < min) min = d[ 5];
            if (d[ 6] < min) min = d[ 6];
            if (d[ 7] < min) min = d[ 7];
            if (d[ 8] < min) min = d[ 8];
            if (d[ 9] < min) min = d[ 9];
            if (d[10] < min) min = d[10];
            if (d[11] < min) min = d[11];
            if (d[12] < min) min = d[12];
            if (d[13] < min) min = d[13];
            if (d[14] < min) min = d[14];
            if (d[15] < min) min = d[15];
            d += 16;
        }
        while(d<df) {
// This is not correct
//            if (*d < min) min = *d;
//            *d++;
// maybe this (compact)
//            if (*d<min) min=*d++;
// surely this
             if (*d<min) min=*d;
             d++;
        }
        return min;
    }

template <class T>
    static inline T __abs_T(T x){
        if (x>0) return x;
        return -x;
    }

template <class T>
    static inline T __max_(T **row,const uint32 n,const uint32 column,uint32 &maxRow,const uint32 initialRow){
        T max = 0;

        for(uint32 i = initialRow; i<n; i++){
            T value = __abs_T(row[i][column]);
            if(max<value){
                max = value;
                maxRow = i;
            }
        }
        return max;
    }

// WARNING: If interested in ussing this library for building
// matrices of general objects, it is necessary to provide
// an element unity and an element zero. Doing static methods of the class T
// like One() and Zero() is a choice, and T.One() and T.Zero() must be used instead
// of 1 and 0 in the code below.

template <class T>
    inline void RTMZerofy_T(RTMatrixT<T> &A){
        T *d=A.data;
        T *df = d + A.DataSize();
        while(d<(df-16)){
            d[ 0] = 0;
            d[ 1] = 0;
            d[ 2] = 0;
            d[ 3] = 0;
            d[ 4] = 0;
            d[ 5] = 0;
            d[ 6] = 0;
            d[ 7] = 0;
            d[ 8] = 0;
            d[ 9] = 0;
            d[10] = 0;
            d[11] = 0;
            d[12] = 0;
            d[13] = 0;
            d[14] = 0;
            d[15] = 0;
            d += 16;
        }
        while(d<df) *d++ = 0;
    }

template <class T>
    inline void RTMOnefy_T(RTMatrixT<T> &A){
        T *d=A.data;
        T *df = d + A.DataSize();
        while(d<(df-16)){
            d[ 0] = 1;
            d[ 1] = 1;
            d[ 2] = 1;
            d[ 3] = 1;
            d[ 4] = 1;
            d[ 5] = 1;
            d[ 6] = 1;
            d[ 7] = 1;
            d[ 8] = 1;
            d[ 9] = 1;
            d[10] = 1;
            d[11] = 1;
            d[12] = 1;
            d[13] = 1;
            d[14] = 1;
            d[15] = 1;
            d += 16;
        }
        while(d<df) *d++ = 1;
    }

template <class T>
    inline void RTMEyefy_T(RTMatrixT<T> &A){
        // First step: zeroing the matrix
        RTMZerofy(A);

        // Second step: putting "1" on the diagonal.
        T* d=A.data;
        T* df=d+A.DataSize();

        uint32 offset=0;
        while(d<df && offset<A.NColumns()){
            *d=1;
            d+=A.NColumns()+1;
            offset++;
        }
    }

template <class T>
    inline void RTMScale_T(RTMatrixT<T> &A,const T factor){
        RTM__vecScale_T(A.data,A.DataSize(),factor);
    }

template <class T>
    inline bool RTMIsNull_T(const RTMatrixT<T> &A){
        return RTM__vecIsNull_T(A.data,A.n*A.m);
    }


template <class Tout,class T1,class T2>
void RTMProduct_T(RTMatrixT<Tout> &rtm,const RTMatrixT<T1> &A,const RTMatrixT<T2> &B){

    Tout *a1= (Tout *)A.row[0]; // beginning of first row of A
    Tout *a2= (Tout *)A.row[rtm.n]; // beginning of last+1 row of A
    Tout *c = (Tout *)rtm.data;
    while (a1 < a2){
        T1 *b1= (T1 *)B.row[0]; // beginning of first column of B
        T1 *b2= (T1 *)b1 + B.m; // beginning of last+1 column of B
        T1 *af= (T1 *)a1 + A.m; // end+1 of current row of A
        while(b1<b2){
            T2 *a  = (T2 *)a1; // running pointer on A row
            T2 *b  = (T2 *)b1; // running pointer on B column
            Tout res = 0; // sum of Arow *Bcol
            // central loop row of A * column of B
            while(a<af){
                res += *a++ * *b;
                b+= B.m;  // move down to next row by adding sizeof row
            }
            *c++ = res; // save value on result & next element
            //
            b1++; // next column
        }
        a1+=A.m; // next row
    }
}

template <class T,class Tin,class Tout>
void RTMVProduct_T(RTMatrixT<T> &rtm,Tin *input,Tout *output){
    Tout *o =  output;
    Tout *oE= &output[rtm.n];
    T *mat  = (T*)rtm.data;
    while (o < oE){
        Tin *iE = &(input[rtm.m]);
        Tin *i= input;
        Tout res = (Tout)0.0;
        while(i<iE) res+=*i++ * *mat++;
        *o++ = res;
    }
}

template <class T,class Tin,class Tout>
void RTMVProductAcc_T(RTMatrixT<T> &rtm,Tin *input,Tout *output){
    Tout *o =  output;
    Tout *oE= &output[rtm.n];
    T *mat  = (T*)rtm.data;
    while (o < oE){
        Tin *iE = &(input[rtm.m]);
        Tin *i= input;
        Tout res = (Tout)0.0;
        while(i<iE) res+=*i++ * *mat++;
        *o++ += res;
    }
}

template <class T1,class T2>
void RTMDotProduct_T(RTMatrixT<T1> &rtm,const RTMatrixT<T2> &A){
    T2 *a1= (T2 *)A.data; // beginning of this
    T2 *a2= a1+rtm.n*rtm.m; // end of this
    T1 *d1= (T1 *)rtm.data;
    while(a1<a2){
        *d1 *= *a1;
        a1++;
        d1++;
    }
}

template <class T>
    T RTMMaxAbsRowSumNorm_T(RTMatrixT<T> &A){
        T currentMax=0;
        T currentSum=0;
        uint32 i;
        uint32 j;
        for (i=0;i<A.NRows();++i){
            for (j=0;j<A.NColumns();++j)
                currentSum+=__abs_T(*(A.data+j+A.NColumns()*i));

            if (currentSum>currentMax)
                currentMax=currentSum;

            currentSum=0;
        }
        return currentMax;
    }

template <class T>
    T RTMMaxAbsColSumNorm_T(RTMatrixT<T> &A){
        T currentMax=0;
        T currentSum=0;
        uint32 i;
        uint32 j;
        for (i=0;i<A.NColumns();++i){
            for (j=0;j<A.NRows();++j)
                currentSum+=__abs_T(*(A.data+i+A.NColumns()*j));

            if (currentSum>currentMax)
                currentMax=currentSum;

            currentSum=0;
        }
        return currentMax;
    }

template <class T>
void RTMTranspose_T(RTMatrixT<T> &rtm,const RTMatrixT<T> &A){
//NOTE: Why typecasting? rtm.data is already of type T* like A.data.
    T* d = (T*)rtm.data;

    T* s1 = (T*)A.data;
    T* sf = s1 + A.m;
    while(s1<sf){
        T* s = s1;
        T* df = d + rtm.m;
        while(d<df){
            *d = *s;
            d++;
            s+=A.m;
        }
        s1++;
    }
}

template <class T1,class T2>
bool RTMInvert_T(RTMatrixT<T1> &rtm,RTMatrixT<T2> &A){

    // This algorithm uses only the culumn pivoting.
    // allocated pivots
    uint32 *pivoted = (uint32 *)malloc(sizeof(uint32)*rtm.n);

    /* Set vector of already pivoted columns to False */
    uint32 *pivotedE = pivoted+rtm.n;
    uint32 *p = pivoted;
    while(p<pivotedE) *p++ = False;

    /* Set destination to Identity */
    T1 *dataE = (T1 *)rtm.row[rtm.n];
    T1 *d     = (T1 *)rtm.data;
    while(d<dataE) *d++ = 0.0;
    d     = rtm.data;
    while(d<dataE) {
        *d = 1.0;
        d+= (rtm.n+1);
    }

    /* main loop to invert the matrix */
    for (int i = 0;i < rtm.n ;i++ ) {
        int newpiv_col = -1;
        int newpiv_row = -1;
        T1 newpiv_absval = 0;

     /* loop to search the pivot: search the maximum absolute value on the matrix */
        int j;
        for (j = 0;j < rtm.n ;j++  ) {
            /* if the row has not been already choosen */
            if (!pivoted[j]) {
                T2 * Arow  = (T2 *)A.row[j];
                for (int k = 0;k < rtm.n ;k++  ) {
                    T2 x = Arow[k];
                    x = __abs_T(x);
                    if (x > newpiv_absval) {
                        /* if the column has not been already choosen and the value is greater */
                        newpiv_absval = x;
                        newpiv_col = k;
                        newpiv_row = j;
                    } /* endif */
                } /* endfor */
            } /* endif */
        } /* endfor */

        /* if the newpiv_absval = 0  then the matrix is singular */
        if (newpiv_absval == 0) {
printf("Singular matrix\n");
            free((void *&)pivoted);
            return False;
        } /* endif */

        /* Set this column done */
        pivoted[newpiv_col] = True;

        /* Set in the destination the pivot inverse value */
        T1 pivot_inv = 1 / A.row[newpiv_row][newpiv_col];

        /* Swap rows in the source so we have pivot in the diagonal */
        /* Also reduce elements in the pivot row */
        for (j = 0;j < rtm.n ;j++  ) {
            T2 temp = A.data[newpiv_row * rtm.n + j];
            A.data[newpiv_row * rtm.n + j] = A.data[newpiv_col * rtm.n + j];
            A.data[newpiv_col * rtm.n + j] = temp * pivot_inv;
        } /* endfor */


        /* Swap rows in the dest so we have pivot in the diagonal */
        /* Also reduce elements in the pivot row */
        for (j = 0;j < rtm.n ;j++  ) {
            T1 temp = rtm.data[newpiv_row * rtm.n + j];
            rtm.data[newpiv_row * rtm.n + j] = rtm.data[newpiv_col * rtm.n + j];
            rtm.data[newpiv_col * rtm.n + j] = temp * pivot_inv;
        } /* endfor */

        /* set to zeroes the columns */
        for (j = 0;j < rtm.n ;j++  ) {
            if (j!= newpiv_col) {
                T2 temp = A.row[j][newpiv_col];
                for (int k = 0;k < rtm.n ;k++  ) {
                    A.row[j][k] -= A.row[newpiv_col][k] * temp;
                    rtm.row[j][k] -= rtm.row[newpiv_col][k] * temp;
                } /* endfor */
            } /* endif */
        } /* endfor */
    } /* endfor */


    free((void *&)pivoted);
    return True;
}


template <class T>
bool RTMLU_T(RTMatrixT<T> &rtm,RTMatrixT<T> &L,RTMatrixT<T> &U,RTMatrixT<T> &P){
    if (rtm.m != rtm.n) return False;       /* The input matrix must be square */
    /* Check of the other matrix */
    if (L.m != rtm.m | L.n != rtm.n) return False;
    if (U.m != rtm.m | U.n != rtm.n) return False;
    if (P.m != rtm.m | P.n != rtm.n) return False;

    /*Initialisation Matrixes */

    int i;
    for(i =0; i<rtm.m;i++){
    /*Initialisation Matrix P = I */
        P.data[i*(rtm.m+1)] = 1;
        L.data[i*(rtm.m+1)] = 1;
    }


    for(i = 0; i<rtm.m*rtm.m;i++){
        U.data[i] = rtm.data[i];
    }

    T  max = 0;
    uint32 maxRow = 0;

    for(int k = 0;k<rtm.m;k++){
        max = __max_(rtm.row,rtm.n,k,maxRow,k);
        if(max==0) return False; //Matrix is singular
        P.SwapRows(k,maxRow);
        U.SwapRows(k,maxRow);
        for(i = k+1;i<rtm.m;i++){
            U.row[i][k] = U.row[i][k]/U.row[k][k];
            for(int j = k+1;j<rtm.m;j++){
                U.row[i][j]=U.row[i][j] - U.row[i][k]*U.row[k][j];
            }
        }
    }

    for(i = 1; i < rtm.m ; i++ ){
        for(int j = 0; j < i ; j++ ){
        L.row[i][j] = U.row[i][j];
        U.row[i][j] = 0;
        }
    }
    return True;
}

#define maximum(a,b) (((a)>(b))?(a):(b))

template <class T>
bool RTMSVD_T(RTMatrixT<T> &rtm,RTMatrixT<T> &U, RTMatrixT<T> &W, RTMatrixT<T> &V)
/* Perform Singular Value Decomposition A = U W Vt
a is replaced by u, w and v must be aldready allocated */
{
    int i, l, k, j, its, nm, jj;
    T c, g, h, s, scale, anorm, f, y, z, x;

//int64 t1,t2;
//T time;

    T *rv1 = (T *)malloc(sizeof(T)*rtm.m);
    T w,u,v;

    RTMCopy(U,rtm);
    g = scale = anorm = 0;

//t1 = Timer::HRTCounter();

    // BLOCK 1 QR decomposition
    { // output => anorm
        T scale = 0;
//        T anorm = 0;
        T g = 0;
        T f;

        int i;
        for(i = 0; i < rtm.m; i++){
            int l = i + 1;
            rv1[i] = scale * g;
            g = 0;
            s = 0;
            scale = 0;
            // only on first n rows
            if(i < rtm.n){
                int k;
                for(k = i; k < rtm.m; k++) scale = scale + __abs_T(U.row[k][i]);
                if(scale != 0){
                    for(k = i; k < rtm.n; k++){
                        U.row[k][i] = U.row[k][i]/scale;
                        s = s + U.row[k][i]*U.row[k][i];
                    }
                    f = U.row[i][i];
                    g = ((f >= 0)?-sqrt(s):sqrt(s));
                    h = f * g - s;
                    U.row[i][i] = f - g;
                    if(i != (rtm.m-1)){
                        for(j = l; j < rtm.m; j++){
                            s = 0;
                            for(k = i; k < rtm.n; k++)  s = s + U.row[k][i] * U.row[k][j];
                            f = s / h;
                            for(k = i; k < rtm.n; k++) U.row[k][j] = U.row[k][j] + f * U.row[k][i];
                        }
                    }
                    for(k = i; k < rtm.n; k++) U.row[k][i] = scale * U.row[k][i];
                }
            }

            W.row[i][i] = scale * g;

            g = s = scale = 0;
            if((i < rtm.n) && (i != rtm.m - 1)){
                int k;
                for(k = l; k < rtm.m; k++) scale = scale + (float)fabs(U.row[i][k]);
                if(scale != 0){
                    for(k = l; k < rtm.m; k++){
                        U.row[i][k] = U.row[i][k] / scale;
                        s = s + U.row[i][k] * U.row[i][k];
                    }
                    f = U.row[i][l];
                    g = (float)((f >= 0)?-sqrt(s):sqrt(s));
                    h = f * g - s;
                    U.row[i][l] = f - g;
                    for(k = l; k < rtm.m; k++) rv1[k] = U.row[i][k] / h;
                    if(i != (rtm.n - 1)){
                        for(j = l; j < rtm.n; j++){
                            s = 0;
                            for(k = l; k < rtm.m; k++)  s = s + U.row[j][k] * U.row[i][k];
                            for(k = l; k < rtm.m; k++)  U.row[j][k] = U.row[j][k] + s * rv1[k];
                        }
                    }
                    for(k = l; k < rtm.m; k++) U.row[i][k] = scale * U.row[i][k];
                }
            }
            anorm = (float)maximum(anorm, (fabs(W.row[i][i]) + fabs(rv1[i])));
        }
    }



//t2 = Timer::HRTCounter();
//t2 -= t1;
//time =t2*Timer::HRTPeriod();
//printf("Time %12le\n",time);
//t1 = Timer::HRTCounter();
    for(i = rtm.m-1; i >= 0; i--)
    {

    if(i < rtm.m-1)
    {
        if(g != 0)
        {
        for(j = l; j < rtm.m; j++)
            V.row[j][i] = (U.row[i][j] / U.row[i][l]) / g;
        for(j = l; j < rtm.m; j++)
        {
            s = 0;
            for(k = l; k < rtm.m; k++)
            s = s + U.row[i][k] * V.row[k][j];
            for(k = l; k < rtm.m; k++)
            V.row[k][j] = V.row[k][j] + s * V.row[k][i];
        }
        }
        for(j = l; j < rtm.m; j++)
        V.row[i][j] = V.row[j][i] = 0;
    }
    V.row[i][i] = 1;
    g = rv1[i];
    l = i;
    }
//t2 = Timer::HRTCounter();
//t2 -= t1;
//time =t2*Timer::HRTPeriod();
//printf("Time2 %12le\n",time);


//t1 = Timer::HRTCounter();
    for(i = rtm.m - 1; i >= 0; i--)
    {
    l = i + 1;
    g = W.row[i][i];
    if(i < rtm.m-1)
    {
        for(j = l; j < rtm.m; j++)
        U.row[i][j] = 0;
    }
    if(g != 0)
    {
        g = 1 / g;
        if(i != rtm.m - 1)
        {
        for(j = l; j < rtm.m; j++)
        {
            s = 0;
            for(k = l; k < rtm.n; k++)
            s = s + U.row[k][i] * U.row[k][j];
            f = (s / U.row[i][i]) * g;
            for(k = i; k < rtm.n; k++)
            U.row[k][j] = U.row[k][j] + f * U.row[k][i];
        }
        }
        for(j = i; j < rtm.n; j++)
        U.row[j][i] = U.row[j][i] * g;
    }
    else
    {
        for(j = i; j < rtm.n; j++)
        U.row[j][i] = 0;
    }
    U.row[i][i] = U.row[i][i] + 1;
    }
//t2 = Timer::HRTCounter();
//t2 -= t1;
//time =t2*Timer::HRTPeriod();
//printf("Time3 %12le\n",time);
//t1 = Timer::HRTCounter();

    for(k = rtm.m - 1; k >= 0; k--)
    {
    for(its = 0; its < 30; its++)
    {
        for(l = k; l >= 0; l--)
        {
        nm = l - 1;
        if( (fabs(rv1[l]) + anorm == anorm) ||
             (fabs(W.row[nm][nm]) + anorm == anorm))
            break;
        }
        if    (its == 30 ||
        ((fabs(rv1[l]) + anorm != anorm) &&
         (fabs(W.row[nm][nm]) + anorm == anorm)))
        {
/* 1 */        c = 0;
        s = 1;
        for(i = l; i <= k; i++)            /* !!!!!!!!!!!!!!!!!!!! */
        {
            f = s * rv1[i];
            if(fabs(f) + anorm != anorm)
            {
            g = W.row[i][i];
            h = (float)sqrt(f * f + g * g);
            W.row[i][i] = h;
            h = 1 / h;
            c = g * h;
            s = - f * h;
            for(j = 0; j < rtm.n; j++)
            {
                y = U.row[j][nm];
                z = U.row[j][i];
                U.row[j][nm] = (y * c) + (z * s);
                U.row[j][i] = -(y * s) + (z * c);
            }
            }
        }
        }
/* 2 */        z = W.row[k][k];
        if(l == k)
        {
        if(z < 0)
        {
            W.row[k][k] = -z;
            for(j = 0; j < rtm.m; j++)
            V.row[j][k] = -V.row[j][k];
        }
        break;
        }
        if(its == 30)
        {
        //printf("No convergence in 30 iterations!\n");
        return -1;
        }
        x = W.row[l][l];
        nm = k - 1;
        y = W.row[nm][nm];
        g = rv1[nm];
        h = rv1[k];
        f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2 * h * y);
        g = (float)sqrt(f * f + 1);
        f = ((x - z) * (x + z) + h * ((y / (f + ((f >= 0)?g:-g))) - h)) / x;
        c = 1;
        s = 1;
        for(j = l; j <= nm; j++)   /* !!!!!!!!!!!!!!!!!!!!!!! */
        {
        i = j + 1;
        g = rv1[i];
        y = W.row[i][i];
        h = s * g;
        g = c * g;
        z = (float)sqrt(f * f + h * h);
        rv1[j] = z;
        c = f / z;
        s = h / z;
        f = (x * c) + (g * s);
        g = -(x * s) + (g * c);
        h = y * s;
        y = y * c;
        for(jj = 0; jj < rtm.m; jj++)
        {
            x = V.row[jj][j];
            z = V.row[jj][i];
            V.row[jj][j] = (x * c) + (z * s);
            V.row[jj][i] = -(x * s) + (z * c);
        }
        z = (float)sqrt(f * f + h * h);
        W.row[j][j] = z;
        if(z != 0)
        {
            z = 1 / z;
            c = f * z;
            s = h * z;
        }
        f = (c * g) + (s * y);
        x = -(s * g) + (c * y);
        for(jj = 0; jj < rtm.n; jj++)
        {
            y = U.row[jj][j];
            z = U.row[jj][i];
            U.row[jj][j] = (y * c) + (z * s);
            U.row[jj][i] = -(y * s) + (z * c);
        }
        }
        rv1[l] = 0;
        rv1[k] = f;
        W.row[k][k] = x;
    }
//printf("its = %i\n",its);
    }
//t2 = Timer::HRTCounter();
//t2 -= t1;
//time =t2*Timer::HRTPeriod();
//printf("Time4 %12le\n",time);
//t1 = Timer::HRTCounter();
    // Sorting of the singular values
    for(i=0;i<rtm.m-1;i++){
        k = i;
        w = W.row[i][i];
        for(j = i+1;j<rtm.m;j++)
            if(W.row[j][j]>=w){
             k=j;
             w=W.row[j][j];
            }
            if(k!=i){
            W.row[k][k] = W.row[i][i];
            W.row[i][i] = w;
            for(j=0;j<rtm.m;j++){
                v = V.row[j][i];
                V.row[j][i] = V.row[j][k];
                V.row[j][k] = v;
            }
            for(j=0;j<rtm.n;j++){
                u = U.row[j][i];
                U.row[j][i] = U.row[j][k];
                U.row[j][k] = u;
            }
            }
    }

    return True;
}




void RTMPrint_I(RTMatrixT<int> &rtm,StreamInterface &s,int maxrow){

    s.Printf("{\n");
    for (int i = 0;i<rtm.n;i++){
        s.Printf("    {\n");
        int cnt = maxrow;
        for (int j = 0;j<rtm.m;j++){
            if (cnt == maxrow) s.Printf("    ");
            s.Printf("%i ",rtm.row[i][j]);
            if (j < (rtm.m-1)) s.Printf(",");
            cnt--;
            if (cnt == 0) s.Printf("\n");
            if (cnt == 0) cnt = maxrow;
        }
        if (cnt != 0) s.Printf("\n");
        if (i < (rtm.n-1)) s.Printf("    },\n");
        else           s.Printf("    }\n}\n");
    }
}

void RTMPrint_F(RTMatrixT<float> &rtm,StreamInterface &s,int maxrow){

    s.Printf("{\n");
    for (int i = 0;i<rtm.n;i++){
        s.Printf("    {\n");
        int cnt = maxrow;
        for (int j = 0;j<rtm.m;j++){
            if (cnt == maxrow) s.Printf("    ");
            s.Printf("%20e ",rtm.row[i][j]);
            if (j < (rtm.m-1)) s.Printf(",");
            cnt--;
            if (cnt == 0) s.Printf("\n");
            if (cnt == 0) cnt = maxrow;
        }
        if (cnt != 0) s.Printf("\n");
        if (i < (rtm.n-1)) s.Printf("    },\n");
        else           s.Printf("    }\n}\n");
    }
}

void RTMPrint_D(RTMatrixT<double> &rtm,StreamInterface &s,int maxrow){

    s.Printf("{\n");
    for (int i = 0;i<rtm.n;i++){
        s.Printf("    {\n");
        int cnt = maxrow;
        for (int j = 0;j<rtm.m;j++){
            if (cnt == maxrow) s.Printf("    ");
            s.Printf("%20e ",rtm.row[i][j]);
            if (j < (rtm.m-1)) s.Printf(",");
            cnt--;
            if (cnt == 0) s.Printf("\n");
            if (cnt == 0) cnt = maxrow;
        }
        if (cnt != 0) s.Printf("\n");
        if (i < (rtm.n-1)) s.Printf("    },\n");
        else           s.Printf("    }\n}\n");
    }
}



template <class T>
bool RTMAllocate_T(RTMatrixT<T> &rtm,uint32 nRows,uint32 nColumns){
    rtm.n = nRows;
    rtm.m = nColumns;
    // allocate main memory
    rtm.data = (T *)malloc(sizeof(T)*rtm.n*rtm.m);
    if (rtm.data == NULL){
        rtm.n = 0;
        rtm.m = 0;
        rtm.row = NULL;
        return False;
    } else {
        // clear all values
        for(int i = 0;i < (rtm.n*rtm.m);i++) rtm.data[i] = 0;

        // allocate pointers to each row beginning and end
        rtm.row = (T **)malloc(sizeof(T*)*(rtm.n+1));
        if (rtm.row == NULL){
            rtm.n = 0;
            rtm.m = 0;
            free((void *&)rtm.data);
            rtm.data = NULL;
            return False;
        } else {
            // create pointers to each row beginning and end
            rtm.row[0] = rtm.data;
            for(int i = 0;i < rtm.n;i++) rtm.row[i+1] = rtm.row[i] + rtm.m;
        }
    }
    return True;
}

template <class T>
void REFMAllocate_T(RefMatrixT<T> &refm){
    if (refm.data != NULL){
        // allocate pointers to each row beginning and end
        refm.row = (T **)malloc(sizeof(T *)*(refm.n+1));
        if (refm.row == NULL){
            refm.n = 0;
            refm.m = 0;
            refm.data = NULL;
        } else {
            // create pointers to each row beginning and end
            refm.row[0] = refm.data;
            for(int i = 0;i < refm.n;i++) refm.row[i+1] = refm.row[i] + refm.m;
        }
    } else REFMDeAllocate(refm);
}

template <class T>
void REFMDeAllocate_T(RefMatrixT<T> &refm){
    if (refm.row != NULL) free((void *&)refm.row);
    refm.row = NULL;
    refm.data = NULL;
    refm.m = 0;
    refm.n = 0;
}

template <class T>
void MAReAllocate_T(MatrixT<T> &ma,uint32 nRows,uint32 nColumns){
    if ((ma.n != nRows)||(ma.m != nColumns)){
        ma.n = nRows;
        ma.m = nColumns;
        if (ma.data!=NULL) free((void *&)ma.data);
        if (ma.row!=NULL) free((void *&)ma.row);
        RTMAllocate(ma,nRows,nColumns);
    }
}




//#######################################################################
//
//          IMPLEMENTATION
//
//#######################################################################

bool RTM__vecIsNull_I(const int*    d,const uint32 sz){return RTM__vecIsNull_T(d,sz);}
bool RTM__vecIsNull_F(const float*  d,const uint32 sz){return RTM__vecIsNull_T(d,sz);}
bool RTM__vecIsNull_D(const double* d,const uint32 sz){return RTM__vecIsNull_T(d,sz);}

void RTM__vecCopy_I (int    *d,const int    *s,const uint32 sz){RTM__vecCopy_T(d,s,sz);}
void RTM__vecCopy_F (float  *d,const float  *s,const uint32 sz){RTM__vecCopy_T(d,s,sz);}
void RTM__vecCopy_D (double *d,const double *s,const uint32 sz){RTM__vecCopy_T(d,s,sz);}
void RTM__vecCopy_DI(double *d,const int    *s,const uint32 sz){RTM__vecCopy_T(d,s,sz);}
void RTM__vecCopy_DF(double *d,const float  *s,const uint32 sz){RTM__vecCopy_T(d,s,sz);}
void RTM__vecCopy_FI(float  *d,const int    *s,const uint32 sz){RTM__vecCopy_T(d,s,sz);}
void RTM__vecCopy_FD(float  *d,const double *s,const uint32 sz){RTM__vecCopy_T(d,s,sz);}
void RTM__vecCopy_IF(int    *d,const float  *s,const uint32 sz){RTM__vecCopy_T(d,s,sz);}
void RTM__vecCopy_ID(int    *d,const double *s,const uint32 sz){RTM__vecCopy_T(d,s,sz);}

void RTM__vecSum_I(int *d,const int *s1,const int *s2,uint32 sz)         {RTM__vecSum_T(d,s1,s2,sz);}
void RTM__vecSum_F(float *d,const float *s1,const float *s2,uint32 sz)   {RTM__vecSum_T(d,s1,s2,sz);}
void RTM__vecSum_D(double *d,const double *s1,const double *s2,uint32 sz){RTM__vecSum_T(d,s1,s2,sz);}

void RTM__vecDiff_I(int *d,const int *s1,const int *s2,uint32 sz)         {RTM__vecDiff_T(d,s1,s2,sz);}
void RTM__vecDiff_F(float *d,const float *s1,const float *s2,uint32 sz)   {RTM__vecDiff_T(d,s1,s2,sz);}
void RTM__vecDiff_D(double *d,const double *s1,const double *s2,uint32 sz){RTM__vecDiff_T(d,s1,s2,sz);}

void RTM__vecComb_I(int *d,uint32 sz,const int *s1,const int w1,const int *s2,const int w2)               { RTM__vecComb_T(d,sz,s1,w1,s2,w2);}
void RTM__vecComb_F(float *d,uint32 sz,const float *s1,const float w1,const float *s2,const float w2)     { RTM__vecComb_T(d,sz,s1,w1,s2,w2);}
void RTM__vecComb_D(double *d,uint32 sz,const double *s1,const double w1,const double *s2,const double w2){ RTM__vecComb_T(d,sz,s1,w1,s2,w2);}

void RTM__vecScale_I(int *d,uint32 sz,const int w){       RTM__vecScale_T(d,sz,w); }
void RTM__vecScale_F(float *d,uint32 sz,const float w){   RTM__vecScale_T(d,sz,w); }
void RTM__vecScale_D(double *d,uint32 sz,const double w){ RTM__vecScale_T(d,sz,w); }

void RTMZerofy_I(RTMatrixT<int>    &A){RTMZerofy_T(A);}
void RTMZerofy_F(RTMatrixT<float>  &A){RTMZerofy_T(A);}
void RTMZerofy_D(RTMatrixT<double> &A){RTMZerofy_T(A);}

void RTMOnefy_I(RTMatrixT<int>    &A){RTMOnefy_T(A);}
void RTMOnefy_F(RTMatrixT<float>  &A){RTMOnefy_T(A);}
void RTMOnefy_D(RTMatrixT<double> &A){RTMOnefy_T(A);}

void RTMEyefy_I(RTMatrixT<int>    &A){RTMEyefy_T(A);}
void RTMEyefy_F(RTMatrixT<float>  &A){RTMEyefy_T(A);}
void RTMEyefy_D(RTMatrixT<double> &A){RTMEyefy_T(A);}

void RTMScale_II(RTMatrixT<int>    &A,const int    factor){RTMScale_T(A,factor);}
void RTMScale_FF(RTMatrixT<float>  &A,const float  factor){RTMScale_T(A,factor);}
void RTMScale_DD(RTMatrixT<double> &A,const double factor){RTMScale_T(A,factor);}

bool RTMIsNull_I(const RTMatrixT<int>    &A){return RTMIsNull_T(A);}
bool RTMIsNull_F(const RTMatrixT<float>  &A){return RTMIsNull_T(A);}
bool RTMIsNull_D(const RTMatrixT<double> &A){return RTMIsNull_T(A);}

int    RTMMax_I(const RTMatrixT<int>    &ma){return RTM__vecMax_T(ma.data,ma.n*ma.m);}
float  RTMMax_F(const RTMatrixT<float>  &ma){return RTM__vecMax_T(ma.data,ma.n*ma.m);}
double RTMMax_D(const RTMatrixT<double> &ma){return RTM__vecMax_T(ma.data,ma.n*ma.m);}

int    RTMMin_I(const RTMatrixT<int>    &ma){return RTM__vecMin_T(ma.data,ma.n*ma.m);}
float  RTMMin_F(const RTMatrixT<float>  &ma){return RTM__vecMin_T(ma.data,ma.n*ma.m);}
double RTMMin_D(const RTMatrixT<double> &ma){return RTM__vecMin_T(ma.data,ma.n*ma.m);}

void RTMCombine_UIIII(RTMatrixT<int>    &rtm,const RTMatrixT<int   > &A,const int    aw,const RTMatrixT<int   > &B,const int    bw){RTMCombine_U(rtm,A,aw,B,bw);}
void RTMCombine_UFFFF(RTMatrixT<float>  &rtm,const RTMatrixT<float > &A,const float  aw,const RTMatrixT<float > &B,const float  bw){RTMCombine_U(rtm,A,aw,B,bw);}
void RTMCombine_UDDDD(RTMatrixT<double> &rtm,const RTMatrixT<double> &A,const double aw,const RTMatrixT<double> &B,const double bw){RTMCombine_U(rtm,A,aw,B,bw);}

void RTMProduct_UIII(RTMatrixT<int>   &rtm,const RTMatrixT<int>   &A,const RTMatrixT<int>   &B){   RTMProduct_T(rtm,A,B); }
void RTMProduct_UFFF(RTMatrixT<float> &rtm,const RTMatrixT<float> &A,const RTMatrixT<float> &B){   RTMProduct_T(rtm,A,B); }
void RTMProduct_UDDD(RTMatrixT<double>&rtm,const RTMatrixT<double>&A,const RTMatrixT<double>&B){   RTMProduct_T(rtm,A,B); }

void RTMVProduct_UIII(RTMatrixT<int> &rtm,int *input,int *output)         { RTMVProduct_T(rtm,input,output); }
void RTMVProduct_UFFF(RTMatrixT<float> &rtm,float *input,float *output)   { RTMVProduct_T(rtm,input,output); }
void RTMVProduct_UDDD(RTMatrixT<double> &rtm,double *input,double *output){ RTMVProduct_T(rtm,input,output); }

void RTMVProductAcc_UIII(RTMatrixT<int> &rtm,int *input,int *output)         { RTMVProductAcc_T(rtm,input,output); }
void RTMVProductAcc_UFFF(RTMatrixT<float> &rtm,float *input,float *output)   { RTMVProductAcc_T(rtm,input,output); }
void RTMVProductAcc_UDDD(RTMatrixT<double> &rtm,double *input,double *output){ RTMVProductAcc_T(rtm,input,output); }

void RTMDotProduct_UII(RTMatrixT<int   > &rtm,const RTMatrixT<int   > &A){ RTMDotProduct_T(rtm,A); }
void RTMDotProduct_UFF(RTMatrixT<float > &rtm,const RTMatrixT<float > &A){ RTMDotProduct_T(rtm,A); }
void RTMDotProduct_UDD(RTMatrixT<double> &rtm,const RTMatrixT<double> &A){ RTMDotProduct_T(rtm,A); }

int    RTMMaxAbsRowSumNorm_I(RTMatrixI &A){return RTMMaxAbsRowSumNorm_T(A);}
float  RTMMaxAbsRowSumNorm_F(RTMatrixF &A){return RTMMaxAbsRowSumNorm_T(A);}
double RTMMaxAbsRowSumNorm_D(RTMatrixD &A){return RTMMaxAbsRowSumNorm_T(A);}

int    RTMMaxAbsColSumNorm_I(RTMatrixI &A){return RTMMaxAbsColSumNorm_T(A);}
float  RTMMaxAbsColSumNorm_F(RTMatrixF &A){return RTMMaxAbsColSumNorm_T(A);}
double RTMMaxAbsColSumNorm_D(RTMatrixD &A){return RTMMaxAbsColSumNorm_T(A);}

void RTMTranspose_UII(RTMatrixT<int   > &rtm,const RTMatrixT<int   > &A){ RTMTranspose_T(rtm,A); }
void RTMTranspose_UFF(RTMatrixT<float > &rtm,const RTMatrixT<float > &A){ RTMTranspose_T(rtm,A); }
void RTMTranspose_UDD(RTMatrixT<double> &rtm,const RTMatrixT<double> &A){ RTMTranspose_T(rtm,A); }

bool RTMInvert_UFF(RTMatrixT<float > &rtm,RTMatrixT<float > &A){ return RTMInvert_T(rtm,A); }
bool RTMInvert_UDF(RTMatrixT<double> &rtm,RTMatrixT<float > &A){ return RTMInvert_T(rtm,A); }
bool RTMInvert_UDD(RTMatrixT<double> &rtm,RTMatrixT<double> &A){ return RTMInvert_T(rtm,A); }

bool RTMLU_FFFF(RTMatrixT<float > &rtm,RTMatrixT<float > &L,RTMatrixT<float > &U,RTMatrixT<float > &P){ return RTMLU_T(rtm,L,U,P); }
bool RTMLU_DDDD(RTMatrixT<double> &rtm,RTMatrixT<double> &L,RTMatrixT<double> &U,RTMatrixT<double> &P){ return RTMLU_T(rtm,L,U,P); }

bool RTMSVD_FFFF(RTMatrixT<float > &rtm,RTMatrixT<float > &U, RTMatrixT<float > &W, RTMatrixT<float > &V){ return RTMSVD_T(rtm,U, W, V); }
bool RTMSVD_DDDD(RTMatrixT<double> &rtm,RTMatrixT<double> &U, RTMatrixT<double> &W, RTMatrixT<double> &V){ return RTMSVD_T(rtm,U, W, V); }

bool RTMAllocate_I(RTMatrixT<int   > &rtm,uint32 nRows,uint32 nColumns){ return RTMAllocate_T(rtm,nRows,nColumns); }
bool RTMAllocate_F(RTMatrixT<float > &rtm,uint32 nRows,uint32 nColumns){ return RTMAllocate_T(rtm,nRows,nColumns); }
bool RTMAllocate_D(RTMatrixT<double> &rtm,uint32 nRows,uint32 nColumns){ return RTMAllocate_T(rtm,nRows,nColumns); }

void REFMAllocate_I(RefMatrixT<int   > &refm){ REFMAllocate_T(refm); }
void REFMAllocate_F(RefMatrixT<float > &refm){ REFMAllocate_T(refm); }
void REFMAllocate_D(RefMatrixT<double> &refm){ REFMAllocate_T(refm); }

void REFMDeAllocate_I(RefMatrixT<int   > &refm){ REFMDeAllocate_T(refm); }
void REFMDeAllocate_F(RefMatrixT<float > &refm){ REFMDeAllocate_T(refm); }
void REFMDeAllocate_D(RefMatrixT<double> &refm){ REFMDeAllocate_T(refm); }

void MAReAllocate_I(MatrixT<int   > &ma,uint32 nRows,uint32 nColumns){ MAReAllocate_T(ma,nRows,nColumns); }
void MAReAllocate_F(MatrixT<float > &ma,uint32 nRows,uint32 nColumns){ MAReAllocate_T(ma,nRows,nColumns); }
void MAReAllocate_D(MatrixT<double> &ma,uint32 nRows,uint32 nColumns){ MAReAllocate_T(ma,nRows,nColumns); }

