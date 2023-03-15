#include <stdio.h>
#include <stdlib.h>

void Print_Matrix(int **M, int rows, int columns) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) 
            printf("%d\t", M[i][j]);
        printf("\n");}
    printf("\n");
    return;
}

void Free_Matrix(int **M, int r){
    // complete this function
    for (int i = 0; i < r; i++) {
	    free(M[i]);
    }
    free(M);    
    return;
}

int ** Create_RxC_Index_Matrix(int r, int c){
    // complete this function
    int ** M = malloc(r * sizeof(int *)); 
    for(int i = 0; i < r; i++) {
        M[i] = malloc(c*sizeof(int)); 
    }

    for (int a = 0; a < r; a++) {
        for(int  b = 0; b < c; b++) {
            M[a][b] = 10 * (a + 1) + (b + 1);
        }
    }
     return M; // replace null with the matrix you create
}

int main() {
    int **M34 = Create_RxC_Index_Matrix(3, 4);
    Print_Matrix(M34, 3, 4);
    Free_Matrix(M34, 3);

    int **M78 = Create_RxC_Index_Matrix(7, 8);
    Print_Matrix(M78, 7, 8);
    Free_Matrix(M78, 7);

    int **M25 = Create_RxC_Index_Matrix(2, 5);
    Print_Matrix(M25, 2, 5);
    Free_Matrix(M25, 2); 

    int **M51 = Create_RxC_Index_Matrix(5, 1);
    Print_Matrix(M51, 5, 1);
    Free_Matrix(M51, 5);

    return 0;
}
