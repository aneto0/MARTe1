#include "GeneralDefinitions.h"



/*	inline T LogicalRightShift(uint8 shift){

		//Observing the size, make unsigned the number and after do right shift.
		switch(sizeof(T)){
			case 1:{
				uint8 numCopy=(uint8) (*number);
				numCopy>>=shift;
				return (T)numCopy;
			}
			break;
			case 2:{
				uint16 numCopy=(uint16)(*number);
				numCopy>>=shift;
				return (T)numCopy;
			}
			break;
			case 4:{
				uint32 numCopy=(uint32)(*number);
				numCopy>>=shift;
				return (T)numCopy;
			}
			break;
			case 8:{
				uint64 numCopy=(uint64)(*number);
				numCopy>>=shift;
				return (T)numCopy;
			}
			break;
		}
 		return (*number);
	}


*/

template <typename T>
class SafeShift{
	
private:
	T* number;
	

private:


	static uint8 LogicalRightShift(uint8 number, uint8 shift){
		return number>>shift;
	}

	static uint16 LogicalRightShift(uint16 number, uint8 shift){
		return number>>shift;
	}

	static uint32 LogicalRightShift(uint32 number, uint8 shift){
		return number>>shift;
	}

	static uint64 LogicalRightShift(uint64 number, uint8 shift){
		return number>>shift;
	}

	static int8 LogicalRightShift(int8 number, uint8 shift){
		return ((uint8)number)>>shift;
	}
	
	static int16 LogicalRightShift(int16 number, uint8 shift){
		return ((uint16)number)>>shift;
	}

	static int32 LogicalRightShift(int32 number, uint8 shift){
		return ((uint32)number)>>shift;
	}

	static int64 LogicalRightShift(int64 number, uint8 shift){
		return ((uint64)number)>>shift;
	}

public:
	template <typename T2>
	static T2 LogicalSafeShift(T2 number, int8 shift){
		int bound = sizeof(T2)*8;
		
		if(shift <= -bound || shift >= bound) return (T2)0;

		if(shift < 0)
			return LogicalRightShift(number,(uint8)(-shift));
		else
			return number<<shift;
	}

	template <typename T2>
	static T2 MathematicSafeShift(T2 number, uint8 shift){ 
		int bound=sizeof(T2)*8;
		//if shift is too much and the number is negative return 0
		if(shift >= bound || shift <= -bound){
			return (T2)0;
		}

		//shift of the number
		if(shift > 0)
			return number<<shift;
		else
			return number>>shift;
	}


public:
	SafeShift(T* genericNumber){
		number = genericNumber;	
	}

	SafeShift(){
		number=NULL;
	}

	void Set(T* genericNumber){
		number = genericNumber;
	}

	//Mathematical right Shift
	T operator>(int8 shift){
	
		if(shift > 0)
			return MathematicSafeShift((*number),-shift);
		else
			return MathematicSafeShift((*number),shift);
					
	}

	//Mathematical left Shift
	T operator<(int8 shift){
		if(shift > 0)
			return MathematicSafeShift((*number),shift);
		else
			return MathematicSafeShift((*number),-shift);

	}


	//Logical right Shift
	T operator>>(int8 shift){

		if(shift > 0)
			return LogicalSafeShift((*number),-shift);
		else
			return LogicalSafeShift((*number),shift);

	}


	//Logical left Shift
	T operator<<(int8 shift){
	
		if(shift > 0)
			return LogicalSafeShift((*number),shift);
		else
			return LogicalSafeShift((*number),-shift);		
		
	}


	//Mathematical right Shift
	void operator>=(int8 shift){

		if(shift > 0)
			(*number) = MatematicSafeShift((*number),-shift);
		else
			(*number) = MatematicSafeShift((*number),shift);
					
	}


	//Mathematical left Shift
	void operator<=(int8 shift){
		if(shift > 0)
			(*number) = MathematicSafeShift((*number),shift);
		else
			(*number) = MathematicSafeShift((*number),-shift);

	}
	


	//Logical right Shift
	void operator>>=(int8 shift){
	
		if(shift > 0)
			(*number) = LogicalSafeShift((*number),-shift);
		else
			(*number) = LogicalSafeShift((*number),shift);		
	}


	void operator<<=(int8 shift){

		if(shift > 0)
			(*number) = LogicalSafeShift((*number),shift);
		else
			(*number) = LogicalSafeShift((*number),-shift);	

		(*number)<<=shift;	
	}	


};
			
		




