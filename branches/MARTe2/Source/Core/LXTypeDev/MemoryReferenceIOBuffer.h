#if !defined (MEMORYREFERENCE_IO_BUFFER)
#define MEMORYREFERENCE_IO_BUFFER

#include "GeneralDefinitions.h"
#include "Memory.h"
#include "IOBuffer.h"


/// Read buffer Mechanism for Streamable
class MemoryReferenceIOBuffer:public IOBuffer {
       
public: // 

	///
	MemoryReferenceIOBuffer()
	{
	}
	
	///
	virtual ~MemoryReferenceIOBuffer();		
   
   
};



#endif
