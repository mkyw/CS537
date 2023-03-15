#include <stdio.h>


int main()
{
   int i = 5;
   void *vptr; 
   vptr = &i;
   printf("Value of iptr = %d\n", *vptr);
   
   return 0;
}
