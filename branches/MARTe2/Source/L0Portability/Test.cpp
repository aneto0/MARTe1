#include <stdio.h>
#include "Processor.h"
#include "HighResolutionTimer.h"

int main(int argc, char **argv){
    Processor p;
    char name[32];
    p.Name(name);
    printf("%s\n", name);
    printf("%d\n", p.Family());
    printf("%d\n", p.Available());

    HighResolutionTimer hrt; 
    printf("%lld\n", hrt.Frequency());
    printf("%e\n", hrt.Period());
}

