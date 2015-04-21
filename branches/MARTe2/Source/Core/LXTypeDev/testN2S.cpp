#include <stdio.h>

typedef unsigned char uint8;

// returns the exponent
// tenToExponent becomes 10 ^ exponent
// positiveNumber is a the abs (number)
template <typename T> uint8 NormalizeInteger(T positiveNumber,T &tenToExponent){       
    tenToExponent = 1;
    T temp ;
    uint8 exp = 0;
    if (sizeof(T)>=8){ // max 19
        temp = tenToExponent * 10000000000; // 10 zeros 
        if (positiveNumber >= temp )  {
            tenToExponent = temp;
            exp += 10;
        }
    }
    if (sizeof(T)>=4){ // max 9 
        temp = tenToExponent * 100000; // 5 zeros
        if (positiveNumber >= temp ) {
            tenToExponent = temp;
            exp += 5;
        }
    }
    if (sizeof(T)>=2){ // max 4 zeros
        temp = tenToExponent * 100; // 2 zeros
        if (positiveNumber >= temp ) {
            tenToExponent = temp;
            exp += 2;
        }

    }
    temp = tenToExponent * 10; // 1 
    if (positiveNumber >= temp ){
            tenToExponent = temp;
            exp ++;
        }
    temp = tenToExponent * 10;  // 1
    if (temp > tenToExponent){
    	if (positiveNumber >= temp ){
            tenToExponent = temp;
            exp ++;
        }
    }
    return exp;
}



class Streamer{

public:
bool PutC(char c){
    putchar(c);
}

}streamer;

/// buffer must have sufficient size to hold a number of size exponent+1! and the trailing zero  
template <typename T> void NumberToDecimalPrivate(char *buffer, T positiveNumber,uint8 exponent){
	char *pCurrent = buffer + exponent + 1;
	*pCurrent-- = 0;
	while (pCurrent >= buffer ){
		unsigned short  digit = positiveNumber % 10;
		positiveNumber        = positiveNumber / 10;
		*pCurrent-- = '0' + digit;
	}
}


template<typename T> void Test(T number){

    for (int i=0; i < (sizeof (T)*2.5);i++){
        T tenToExponent;
        uint8 exp = NormalizeInteger(number,tenToExponent);
        char buffer[80];

        NumberToDecimalPrivate(buffer,number,exp);
printf("(%2i) %s ",i, buffer);
        NumberToDecimalPrivate(buffer,tenToExponent,exp);
printf(" %s  %02i\n",buffer, exp);
//        if (sizeof(T)==1) printf("C(%2i) % 20i % 20i  %02i\n",i,number, tenToExponent, exp);
//        if (sizeof(T)==2) printf("S(%2i) % 20i % 20i  %02i\n",i,number, tenToExponent, exp);
//        if (sizeof(T)==4) printf("I(%2i) % 20li % 20li  %02i\n",i,number, tenToExponent, exp);
//       if (sizeof(T)==8) printf("L(%2i) % 20Li % 20Li  %02i\n",i,number, tenToExponent, exp);

        number *= 10;

    }
}



int main(){
int N = 1;

Test((unsigned char)N);
Test((unsigned short)N);
Test((unsigned int)N);
Test((unsigned long long)N);
Test((char)N);
Test((short)N);
Test((int)N);
Test((long long)N);

return 0;

}