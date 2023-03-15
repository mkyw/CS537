#include <stdio.h>
// functions f0 - f4 were defined here 
// The code has been removed so you will 
// need to disassemble the executable or use gdb 
int main() {
    // parameters
    int p1 = 2802;
    int p2 = 3;
    int p3 = 13;
    int p4 = 16;
    int p5 = 48;
    // compute the values assigned to the array
    int arr[5];
    int *p_arr = arr;
    *(p_arr+0) = f0();
    *(p_arr+1) = f1(p3, p1);
    *(p_arr+2) = f2(p2, p3);
    *(p_arr+3) = f3(p4, p3);
    f4(&arr[4], p5, p1);
    
//     // print the results
//     for (int i = 0; i<5; i++) 
//         printf("arr[%d] = %d\n",i,arr[i]);
    return 0;
}
