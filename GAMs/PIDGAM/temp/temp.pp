# 1 "PIDGAMClassInfo.h"
# 1 "/home/andre/Projects/EFDA-MARTe/GAMs/PIDGAM//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "PIDGAMClassInfo.h"
# 28 "PIDGAMClassInfo.h"
# 1 "../../BaseLib2/Level0/System.h" 1
# 51 "../../BaseLib2/Level0/System.h"
# 1 "../../BaseLib2/Level0/SystemCINT.h" 1
# 36 "../../BaseLib2/Level0/SystemCINT.h"
struct unknown{
    void *nowhere;
};
# 52 "../../BaseLib2/Level0/System.h" 2


# 1 "../../BaseLib2/Level0/GenDefs.h" 1
# 38 "../../BaseLib2/Level0/GenDefs.h"
typedef float real;
# 175 "../../BaseLib2/Level0/GenDefs.h"
enum Colours{

    Black = 0,

    DarkBlue = 1,

    DarkGreen = 2,

    DarkCyan = 3,

    DarkRed = 4,

    DarkPurple = 5,

    DarkYellow = 6,

    Grey = 7,

    DarkGrey = 8,

    Blue = 9,

    Green = 10,

    Cyan = 11,

    Red = 12,

    Purple = 13,

    Yellow = 14,

    White = 15
};
# 55 "../../BaseLib2/Level0/System.h" 2
# 1 "../../BaseLib2/Level0/Memory.h" 1
# 32 "../../BaseLib2/Level0/Memory.h"
# 1 "../../BaseLib2/Level0/System.h" 1
# 33 "../../BaseLib2/Level0/Memory.h" 2


class StreamInterface;


enum MemoryAllocationFlags{

    MEMORYStandardMemory = 0x00000000,


    MEMORYExtraMemory = 0x00000001,
};


enum MemoryTestAccessMode{

    MTAM_Execute = 0x00000001,


    MTAM_Read = 0x00000002,


    MTAM_Write = 0x00000004

};


static inline MemoryTestAccessMode operator &(MemoryTestAccessMode a, MemoryTestAccessMode b){
    return (MemoryTestAccessMode) ((int)a & (int) b);
}


static inline MemoryTestAccessMode operator |(MemoryTestAccessMode a, MemoryTestAccessMode b){
    return (MemoryTestAccessMode) ((int)a | (int) b);
}

extern "C" {






    void *MEMORYMalloc(int size,MemoryAllocationFlags allocFlags=MEMORYStandardMemory);




    void MEMORYFree(void *&data);






    void *MEMORYRealloc(void *&data,int newSize);





    char *MEMORYStrDup(const char *s);




    void MEMORYDisplayAllocationStatistics(StreamInterface *out);







    bool MEMORYAllocationStatistics(int &size, int &chunks, long tid = (long)0xFFFFFFFF);







    bool MEMORYCheck(void *address, MemoryTestAccessMode accessMode,int size=4);
# 125 "../../BaseLib2/Level0/Memory.h"
    void *SharedMemoryAlloc(unsigned int key, unsigned int size, unsigned int permMask = 0666);




    void SharedMemoryFree(void *address);


}
# 56 "../../BaseLib2/Level0/System.h" 2




extern "C"{

    int UserMainFunction(int argc,char **argv);




    int MainHandler(int (*userMainFunction)(int argc,char **argv),int argc,char **argv);
}
# 29 "PIDGAMClassInfo.h" 2
# 1 "PIDGAMInputStructure.h" 1
# 30 "PIDGAMInputStructure.h"
struct PIDGAMInputStructure {

    unsigned int usecTime;

    float reference;

    float measurement;

    float feedforward;
};
# 30 "PIDGAMClassInfo.h" 2
# 1 "PIDGAMOutputStructure.h" 1
# 30 "PIDGAMOutputStructure.h"
struct PIDGAMOutputStructure {

    float controlSignal;

    float feedback;

    float error;

    float integratorState;

    float fastDischargeAction;

    float antiwindupAction;
};
# 31 "PIDGAMClassInfo.h" 2

struct PIDGAMClassInfo {
    PIDGAMInputStructure input;
    PIDGAMOutputStructure output;
};
