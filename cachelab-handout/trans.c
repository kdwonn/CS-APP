/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int block_row, block_col;
    int row, col;
    int r, temp, temp2;
    int x0, x1, x2, x3, x4;

   if(N == 32){
     for(block_col = 0; block_col < M; block_col += 8){
        for(block_row = 0; block_row < N; block_row += 8){
            for(row = block_row; row < block_row + 8; row++){
                for(col = block_col; col < block_col + 8; col++){
                    if(col != row)
                        B[col][row] = A[row][col];
                    else{
                        temp = A[row][col];
                        r = row;
                    }
                }
                if(block_col == block_row)
                    B[r][r] = temp;
            }
        }
    }
   }
   else if(N == 64){
    for(row = 0; row < N; row += 4) 
    { 
       for(col = 0; col < M; col += 4) 
       { 
           x0 = A[row][col]; 
           x1 = A[row+1][col]; 
           x2 = A[row+2][col]; 
           x3 = A[row+2][col+1]; 
           x4 = A[row+2][col+2]; 

           B[col+3][row] = A[row][col+3]; 
           B[col+3][row+1] = A[row+1][col+3]; 
           B[col+3][row+2] = A[row+2][col+3]; 

           B[col+2][row] = A[row][col+2]; 
           B[col+2][row+1] = A[row+1][col+2]; 
           B[col+2][row+2] = x4; 
           x4 = A[row+1][col+1]; 

           B[col+1][row] = A[row][col+1]; 
           B[col+1][row+1] = x4; 
           B[col+1][row+2] = x3; 

           B[col][row] = x0; 
           B[col][row+1] = x1; 
           B[col][row+2] = x2; 

           B[col][row+3] = A[row+3][col]; 
           B[col+1][row+3] = A[row+3][col+1]; 
           B[col+2][row+3] = A[row+3][col+2]; 
           x0 = A[row+3][col+3]; 

           B[col+3][row+3] = x0; 
           
           /*
           if(block_col == 58){
            if(block_row != 58){
                for(i = 0; i < 4; i ++){
                    for(j = 0; j < 8; j ++){
                        B[0 + i][block_row + 8 + j] = A[block_row + 4 + i][block_col + j];
                    }
                }
                for(i = 0; i < 8; i ++){
                    for(j = 0; j < 4; j ++){
                        B[block_col + i][block_row + j] = A[block_row + j][block_col + i];
                        B[block_col + i][block_row + j + 4] = B[0 + j][block_row + i];
                    }
                }
            }
            else{
                for(i = 0; i < 8; i ++){
                    for(j = 0; j < 8; j ++){
                        B[block_col + j][block_row + i] = A[block_row + i][block_col + j];
                    }
                }
            }
           }
           else{
            for(i = 0; i < 4; i ++){
                for(j = 0; j < 8; j ++){
                    B[block_col + 8 + i][block_row + j] = A[block_row + 4 + i][block_col + j];
                }
            }
            for(i = 0; i < 8; i ++){
                for(j = 0; j < 4; j ++){
                    B[block_col + i][block_row + j] = A[block_row + j][block_col + i];
                    B[block_col + i][block_row + j + 4] = B[block_col + 8 + j][block_row + i];
                }
            }
           }*/
           //generate buffer for bottom half of 8*8 block of A
           

        } 
    } 
   }
   else{
    for(block_col = 0; block_col < M; block_col += 7){
        for(block_row = 0; block_row < N; block_row += 7){
            for(row = block_row; (row < block_row + 7) && (row < N); row++){
                for(col = block_col; (col < block_col + 7) && (col < M); col++){
                    if(block_row == block_col){
                        if(row - block_row == 0 && col - block_col == 1){
                            temp2 = A[row][col];
                            continue;
                        }
                    }
                    if(col != row)
                        B[col][row] = A[row][col];
                    else{
                        temp = A[row][col];
                        r = row;
                    }
                }
                if(block_col == block_row)
                    B[r][r] = temp;
            }
            if(block_row == block_col)
                B[block_col + 1][block_row] = temp2;
        }
    }
   }
}

char second_func_desc[] = "Second function";
void second_func(int M, int N, int A[N][M], int B[M][N]){

}

char third_func_desc[] = "third_function";
void third_func(int M, int N, int A[N][M], int B[M][N]){
    int block_row, block_col;
    int  i, j;

    for(block_row = 0; block_row < N; block_row += 8) 
    { 
       for(block_col = 0; block_col < M; block_col += 8) 
       { 
        if(block_col == 58){
            if(block_row != 58){
                for(i = 0; i < 4; i ++){
                    for(j = 0; j < 8; j ++){
                        B[0 + i][block_row + 8 + j] = A[block_row + 4 + i][block_col + j];
                    }
                }
                for(i = 0; i < 8; i ++){
                    for(j = 0; j < 4; j ++){
                        B[block_col + i][block_row + j] = A[block_row + j][block_col + i];
                        B[block_col + i][block_row + j + 4] = B[0 + j][block_row +i];
                    }
                }
            }
            else{
                for(i = 0; i < 8; i ++){
                    for(j = 0; j < 8; j ++){
                        B[block_col + j][block_row + i] = A[block_row + i][block_col + j];
                    }
                }
            }
           }
           else{
            for(i = 0; i < 4; i ++){
                for(j = 0; j < 8; j ++){
                    B[block_col + 8 + i][block_row + j] = A[block_row + 4 + i][block_col + j];
                }
            }
            for(i = 0; i < 8; i ++){
                for(j = 0; j < 4; j ++){
                    B[block_col + i][block_row + j] = A[block_row + j][block_col + i];
                    B[block_col + i][block_row + j + 4] = B[block_col + 8 + j][block_row + i];
                }
            }
           }
       }
   }
}



/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 
    registerTransFunction(third_func, third_func_desc);

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

