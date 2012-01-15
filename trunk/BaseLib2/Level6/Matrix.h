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
 * @brief Matrix implementation
 *
 * Sanity boundary checks are performed
 */
#include "RTMatrix.h"
#include "CDBExtended.h"

#if !defined(_MATRIX_H_)
#define _MATRIX_H_

template <class T>
class MatrixT;


/** copies a matrix */
template <class T1,class T2>
inline bool RTMCopy(MatrixT<T1> &dest, const MatrixT<T2> &src){
    if (dest.m != src.m) return False;
    if (dest.n != src.n) return False;

    uint32 sz = src.DataSize();
    RTM__vecCopy(dest.data,src.data,sz);

    return True;
}

/** loads a packed array of data as floats */
template <class T1,class T2>
inline bool RTMLoad(MatrixT<T1> &dest,const T2 *src, const uint32 nRows,const uint32 nColumns){
    if (dest.m != nColumns) return False;
    if (dest.n != nRows) return False;

    uint32 sz = dest.DataSize();
    RTM__vecCopy(dest.data,src,sz);

    return True;
}

/** saves the matrix into a packed array of data */
template <class T1,class T2>
inline bool RTMSave(MatrixT<T1> &src,T2 *dest, const uint32 nRows,const uint32 nColumns){
    if (src.m != nColumns) return False;
    if (src.n != nRows) return False;

    uint32 sz = src.DataSize();
    RTM__vecCopy(dest,src.data,sz);
    return True;
}

#if defined (MATRIX_BOUNDARY_CHECK)
    static char matrixScratch[16];
#endif

/** a row of th ematrix */
template <class T>
class MatrixRow{
    /** the actual storage */
    T *row;
    /** n of columns */
    int m;
public:
    /** */
    inline MatrixRow(T *row,int m){
        this->row = row;
        this->m   = m;
    }
    /** */
    inline T &operator[](int col)const{
#if defined (MATRIX_BOUNDARY_CHECK)
        if  ((col<0) || (col>=m)){
            CStaticAssertErrorCondition(ParametersError,"Matrix boundary violation: (row ptr=%x) column index = %i [0:%i)",row,col,m);
            return (T &)matrixScratch;
        }
        if (row == NULL)
            return (T &)matrixScratch;
#endif
        return row[col];
    }
};


/** this is an extension of RTMatrix with algebraic capabilities */
template <class T>
class MatrixT: public RTMatrixT<T> {
public:

    /** */
    void ReSize(uint32 nRows,uint32 nColumns){
        MAReAllocate(*this,nRows,nColumns);
    }

    /** */
    inline MatrixT(){
        this->Initialize();
    }

    /** */
    inline MatrixT(uint32 nRows,uint32 nColumns){
        this->Allocate(nRows,nColumns);
    }

    /** */
    inline MatrixT(uint32 nRows,uint32 nColumns,const T *source){
        this->Allocate(nRows,nColumns);
        uint32 sz = this->DataSize();
        RTM__vecCopy(this->data,source,sz);
    }

    /** */
    inline MatrixT(const RTMatrixT<T> &A){
        *this = A;
    }

    /** */
    inline MatrixT(const MatrixT &A){
        *this = A;
    }

    /** */
    inline ~MatrixT(){
        this->DeAllocate();
    }

    /** */
    inline MatrixT &operator=(const RTMatrixT<T> &A){
        MAReAllocate(*this,A.NRows(),A.NColumns());
        uint32 sz = A.DataSize();
        RTM__vecCopy(this->data,A.data,sz);
        return *this;
    }

    /** */
    inline MatrixT &operator=(const MatrixT<T> &A){
        MAReAllocate(*this,A.NRows(),A.NColumns());
        uint32 sz = A.DataSize();
        RTM__vecCopy(this->data,A.data,sz);
        return *this;
    }

    /** Allow fast access to elements */
    inline const MatrixRow<T> operator[](int rowNo){
#if defined (MATRIX_BOUNDARY_CHECK)
        if ((rowNo <0) || (rowNo>=n)){
            CStaticAssertErrorCondition(ParametersError,"Matrix %x boundary violation: row index = %i [0:%i) ",this,rowNo,n);
            return MatrixRow<T>(NULL,m);
        }
#endif
        return MatrixRow<T>(this->row[rowNo],this->m);
    }

    /** */
    inline bool operator*=(const RTMatrixT<T> &A){
        MatrixT temp = *this;
        MAReAllocate(*this,temp.NRows(),A.NColumns());
        RTMProduct_U(*this,temp,A);
        return True;
    }

    /** */
    inline void operator*=(T x){
        Scale(x);
    }

    /** */
    inline bool const operator+=(const RTMatrixT<T> &A){
        MAReAllocate(*this,A.NRows(),A.NColumns());
        RTMSum_U(*this,*this,A);
        return True;
    }

    /** -= */
    inline bool operator-=(const RTMatrixT<T> &A){
        MAReAllocate(*this,A.NRows(),A.NColumns());
        RTMDiff_U(*this,*this,A);
        return True;
    }

};



/** */
template<class T>
inline const MatrixT<T> operator*(const RTMatrixT<T> &A,const RTMatrixT<T> &B){
    MatrixT<T> res;
    if (A.NColumns() != B.NRows()) return res;
    MAReAllocate(res,A.NRows(),B.NColumns());
    RTMProduct_U(res,A,B);
    return res;
}

/** */
template<class T>
inline const MatrixT<T> operator*(const RTMatrixT<T> &A,T x){
    MatrixT<T> res = A;
    res.Scale(x);
    return res;
}

/** */
template<class T>
inline const MatrixT<T> operator*(T x,const RTMatrixT<T> &A){
    MatrixT<T> res = A;
    res.Scale(x);
    return res;
}

/** */
template<class T>
inline const MatrixT<T> operator+(const RTMatrixT<T> &A,const RTMatrixT<T> &B){
    MatrixT<T> res(A.NRows(),A.NColumns());
    RTMSum_U(res,A,B);
    return res;
}

/** difference */
template<class T>
inline const MatrixT<T> operator-(const RTMatrixT<T> &A,const RTMatrixT<T> &B){
    MatrixT<T> res(A.NRows(),A.NColumns());
    RTMDiff_U(res,A,B);
    return res;
}

/**  A^-1 * a */
template<class T>
const MatrixT<T> operator/(T a,const RTMatrixT<T> &A){
    MatrixT<T> res;
    if (A.NRows() != A.NColumns()) return res;
    MAReAllocate(res,A.NRows(),A.NColumns());
    MatrixT<T> temp=A;
    if (RTMInvert_U(res,temp)==False){
        res.DeAllocate();
        return res;
    }
    res.Scale(a);
    return res;
}

/** ratio of A* B^-1 */
template<class T>
inline const MatrixT<T> operator/(const RTMatrixT<T> &A,const RTMatrixT<T> &B){
    MatrixT<T> res;
    if (B.NRows() != B.NColumns()) return res;
    if (A.NColumns() != B.NRows()) return res;
    MatrixT<T> temp=B;
    MatrixT<T> temp2(B.NRows(),B.NColumns());
    if ( RTMInvert_U(temp2,temp) == False){
        res.DeAllocate();
        return res;
    }
    MAReAllocate(res,A.NRows(),A.NColumns());
    RTMProduct_U(res,A,temp2);
    return res;
}

/** transpose */
template<class T>
inline const MatrixT<T> operator~(const RTMatrixT<T> &A){
    MatrixT<T> res;
    MAReAllocate(res,A.NColumns(),A.NRows());
    RTMTranspose_U(res,A);
    return res;
}


/** */
#define MatrixI MatrixT<int   >

/** */
#define MatrixF MatrixT<float >

/** */
#define MatrixD MatrixT<double>

/** */
#define rows_of(a) (sizeof(a)/sizeof(a[0]))

/** */
#define columns_of(a) (sizeof(a[0])/sizeof(a[0][0]))

/** */
inline bool ReadRTMatrixF(ConfigurationDataBase &cdb_,MatrixF &matrix,const char *configName){
    CDBExtended &cdb = (CDBExtended &)cdb_;

    int size[4];
    int maxDim = 4;
    if (!cdb->GetArrayDims(size,maxDim,configName,CDBAIM_Strict)) return False;
    if (maxDim > 2){
//            CStaticAssertErrorCondition(-1,"ReadRTMatrixF: too many dimensions to load into RTMatrix");
        return False;
    }
    if (size[0] == 0) size[0] = 1;
    if (size[1] == 0) size[1] = 1;
    matrix.ReSize(size[0],size[1]);
    return cdb.ReadFloatArray(matrix.data,size,maxDim,configName);
}

#endif




