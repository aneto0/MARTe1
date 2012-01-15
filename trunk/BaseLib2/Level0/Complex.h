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
 * an implementation of the complex numerical type as 2 doubles
 */
#ifndef _COMPLEX_H_
#define _COMPLEX_H_

class Complex{

    /** */
    double real;

    /** */
    double imaginary;

public:
    /** */
    inline Complex(){ }

    /** 
     * Constructor from a double real value
     * @param x the real value to set
     */
    inline Complex(double x){
        real = x;
        imaginary = 0;
    }

    /** 
     * Constructor from an integer real value
     * @param x the real value to set 
     */
    inline Complex(int x){
        real = x;
        imaginary = 0;
    }

    /** 
     * Constructor from a double real value
     * and a double imaginary value
     * @param r the real value to set 
     * @param i the imaginary value to set 
     */
    inline Complex(double r,double i){
        real = r;
        imaginary = i;
    }

    /** 
     * Constructor from an array, where the first index 
     * contains the real value and the second index the 
     * imaginary value
     * @param r the real and complex value 
     */
    inline Complex(double r[2]){
        real = r[0];
        imaginary = r[1];
    }
   
    /**
     * Copy constructor. This complex well get 
     * the real and imaginary parts from the argument
     * @param x the complex value to be copied
     */
    inline Complex(const Complex &x){
        real = x.real;
        imaginary = x.imaginary;
    }

    /** 
     * @return the real part of the complex
     */
    inline double Real(){ return real;}

    /** 
     * @return the imaginary part of the complex
     */
    inline double Imaginary(){ return imaginary;}

    //For RTAI not to crash, must return Complex&
    /** 
     * = operator. It will copy both the real and 
     * imaginary parts from the argument
     * @param x the parameter to be copied
     */
    inline Complex& operator=(const Complex &x){
        real = x.real;
        imaginary = x.imaginary;        
        return *this;
    }

    /** 
     * = operator. The real part will be equal to the
     * argument and the imaginary part will be 0
     * @param x the real part to be assigned 
     */
    inline Complex& operator=(double x){
        real = x;
        imaginary = 0;
        return *this;
    }

    /** 
     * = operator. The real part will be equal to the
     * argument and the imaginary part will be 0
     * @param x the real part to be assigned 
     */
    inline Complex& operator=(int x){
        real = x;
        imaginary = 0;
        return *this;
    }

    /** 
     * Performs the complex conjugate 
     */
    inline friend const Complex operator~(const Complex &x){
        Complex ret;
        ret.real = x.real;
        ret.imaginary = -x.imaginary;
        return ret;
    }

    inline friend const Complex operator-(const Complex &x){
        Complex ret;
        ret.real = -x.real;
        ret.imaginary = -x.imaginary;
        return ret;
    }

    inline friend const Complex operator+(const Complex &a,const Complex &b){
        Complex ret(a.real+b.real,a.imaginary+b.imaginary);
        return ret;
    }

    inline friend const Complex operator+(const Complex &a,double b){
        Complex ret(a.real+b,a.imaginary);
        return ret;
    }

    inline friend const Complex operator+(double a,const Complex &b){
        Complex ret(a+b.real,b.imaginary);
        return ret;
    }

    inline void operator +=(const Complex &a){
        real+=a.real;
        imaginary += a.imaginary;
    }

    inline void operator +=(double a){
        real+=a;
    }

    inline friend const Complex operator-(const Complex &a,const Complex &b){
        Complex ret(a.real-b.real,a.imaginary-b.imaginary);
        return ret;
    }

    inline friend const Complex operator-(double a,const Complex &b){
        Complex ret(a-b.real,b.imaginary);
        return ret;
    }

    inline friend const Complex operator-(const Complex &a,double b){
        Complex ret(a.real-b,a.imaginary);
        return ret;
    }

    inline void operator -=(const Complex &a){
        real -= a.real;
        imaginary -= a.imaginary;
    }

    inline void operator -=(double a){
        real-=a;
    }

    inline friend const Complex operator*(const Complex &a,const Complex &b){
        Complex ret(a.real*b.real-a.imaginary*b.imaginary,a.real*b.imaginary+b.real*a.imaginary);
        return ret;
    }

    inline friend const Complex operator*(const Complex &a,double b){
        Complex ret(a.real*b,a.imaginary*b);
        return ret;
    }

    inline friend const Complex operator*(double a,const Complex &b){
        Complex ret(b.real*a,b.imaginary*a);
        return ret;
    }

    inline void operator *=(const Complex &a){
        *this = *this * a;
    }

    inline void operator *=(double a){
        real *= a;
        imaginary *= a;
    }

    inline friend const Complex operator/(const Complex &a,const Complex &b){
        Complex ret(a);
        ret /= b.Norma2();
        ret *= ~b;
        return ret;
    }

    inline friend const Complex operator/(const Complex &a,double b){
        Complex ret = a;
        ret /= b;
        return ret;
    }

    inline friend const Complex operator/(double a,const Complex &b){
        Complex ret = a;
        ret /= b.Norma2();
        ret *= ~b;
        return ret;
    }

    inline void operator /=(const Complex &a){
        *this = *this / a;
    }

    inline void operator /=(double a){
        real/=a;
        imaginary/=a;
    }

    /**
     * Compares using the Norma2()
     */
    inline bool operator < (const Complex &x){
        return (Norma2() < x.Norma2());
    }

    /**
     * Compares using the Norma2()(Square of the norm)
     */
    inline bool operator > (const Complex &x){
        return (Norma2() > x.Norma2());
    }

    /**
     * Both the real and imaginary parts must be equal
     */
    inline bool operator == (const Complex &x){
        return ((real == x.real) && (imaginary == x.imaginary));
    }

    /**
     * The real parts are equal and the imaginary part is zero 
     */
    inline bool operator == (double x){
        return ((real == x) && (imaginary == 0));
    }

    /**
     * Square of the norm
     */
    inline double Norma2() const{ 
        return real*real+imaginary*imaginary;
    }

    /**
     * Norm of the complex
     */
    inline double Norma()const { return sqrt(Norma2());  }

    /**
     * Computes the complex argument (or phase)
     * @return the complex argument
     */
    inline double Arg()const {
        if (real == 0){
            if (imaginary>0){
                return 1.570704;
            }
            return -1.570704;
        }
        double arg = atan(imaginary/real);
        if(real>0){
            return arg;
        }
        else if(imaginary>0){
            return -3.1415 + arg;
        }
        else{
            return  3.1415 + arg;
        }
        
    }

    inline friend const Complex sin(const Complex &x){
        return -1*(cos(x.real)*sinh(x.imaginary)+sin(x.real)*cosh(x.imaginary));
    }

    inline friend const Complex cos(const Complex &x){
        return sin(x.real)*sinh(x.imaginary) - cos(x.real)*cosh(x.imaginary);
    }

    inline friend const Complex exp(const Complex &x){
        return Complex(exp(x.real)*cos(x.imaginary),exp(x.real) * sin(x.imaginary));
    }

    inline friend const Complex log(const Complex &x){
        return Complex(log(x.Norma()),x.Arg());
    }

    inline friend const Complex Clog(double x){
        if (x>0) return Complex(log(x),0);
        else return Complex(log(-x),3.1415);
    }

    inline friend const Complex sqrt(const Complex &x){
        double norma = sqrt(x.Norma());
        double arg   = x.Arg()/2;

        return Complex(norma*cos(arg),norma*sin(arg));
    }

    inline friend const Complex Csqrt(double x){
        if (x>0) return Complex(sqrt(x),0);
        return Complex(0,sqrt(-x));
    }

    inline friend const Complex sqr(const Complex &x){
        return Complex(x.real*x.real-x.imaginary*x.imaginary,x.real*x.imaginary+x.real*x.imaginary);
    }
};

#endif

