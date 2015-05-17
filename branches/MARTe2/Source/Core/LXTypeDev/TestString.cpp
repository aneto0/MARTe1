#include "StreamString.h"
#include <stdio.h>

int main(){
	printf("TEST\n");
	
	StreamString s;
	printf("buffer = %x\n",s.Buffer());
	printf("size = %i\n",s.Size());
	
	s = "Ciao begli stronzi";
	
	printf("TEST\n");
	printf("size = %i\n",s.Size());
	printf("value = %s\n",s.Buffer());
	
	s.Seek(0);
        s.PutC('H');
	printf("value = %s\n",s.Buffer());

	s.Seek(20);
        s.PutC('?');
	printf("value = %s\n",s.Buffer());
	
	s.Seek(0);
        s.Printf("woopah = %i\n",32768);
        s.Printf("o = (%f,%5.3f) \n",2.1,0.2);

        s.Printf("address of s = (%p) \n",&s);

        const char *constantChar= "ULLALAH!";

        s.Printf("A constant char (%7s) \n",constantChar);

        s.Printf("The pointer to a constant char  (%p) \n",(void *)constantChar);
        s.Printf("A constant char as a pointer (%x) \n",constantChar);

	printf("value = %s\n",s.Buffer());


	return 0;
}