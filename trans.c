/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 Ben Reichert
 CS login: breic2
 referenced: https://github.com/yichuns/CacheSimulator/blob/master/trans.c~

 */ 
#include <stdio.h>
#include "cachelab.h"
#include <stdlib.h>

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
	if(M == 0 || N == 0){
		printf("please dont try to multiple matrixes by 0.\n");
		abort();
	}
	
	//use the blocking, Luke!
	int blocksize = 0, blockrow = 0, blockcolumn = 0, j = 0, i = 0, temp = 0, s = 0, d = 0;
	/*
	d = diagonal index of a referenced item
	s = stores status of accessed element
	blocksize = how big we want to block, usually a subset of the whole [N][M] matrix
	blockrow = 

	using the idea of blocking to break up the data so that when it is accessed by the cache it is already near the data it needs
	*/
	
	//32x32 matrix
	if(M == 32 && N == 32){
	blocksize = 8; //after experimenting with different block sizes, 8 works best to reduce misses

		for(blockrow = 0; blockrow < N; blockrow += blocksize){
			for(blockcolumn = 0; blockcolumn < M; blockcolumn += blocksize){
				for(i = blockrow; i < blockrow + blocksize; i++){
					for(j = blockcolumn; j < blockcolumn + blocksize; ++j){
						if(j != i){ //these are the diagonals, so dont touch them
							//if j and i are the same its a diagonal and do that in the else statement
							B[j][i] = A[i][j]; //if it is not a diagonal, just transpose that thing
						}
						else{
							temp = A[i][j]; //save the value in a temp, so that we dont fuck up the locality of the cache at that point
							d = i; //save the diagonal index so we can transpose it after this loop, like storing datas
							s = 1; //set the status so that we know to translate the diagonal element
						}
					}
					//now we need to transpose the elements when the status bit is set
					if(s == 1){
						B[d][d] = temp; //assign the temp now that we're done with that cache section
						s = 0;
					}
				}
			}
		}	
	}

	//64x64 matrix
	if(M == 64 && N == 64){
	blocksize = 4; //after experimenting with different block sizes, 4 works best to reduce misses (and its still a lot)

		for(blockcolumn = 0; blockcolumn < N; blockcolumn += blocksize){
			for(blockrow = 0; blockrow < M; blockrow += blocksize){
				for(i = blockrow; i < blockrow + blocksize; i++){
					for(j = blockcolumn; j < blockcolumn + blocksize; ++j){
						if(i != j){ //these are the diagonals, so dont touch them
							//if j and i are the same its a diagonal and do that in the else statement
							B[j][i] = A[i][j]; //if it is not a diagonal, just transpose that thing
						}
						else{
							temp = A[i][j]; //save the value in a temp, so that we dont fuck up the locality of the cache at that point
							d = i; //save the diagonal index so we can transpose it after this loop, like storing datas
							s = 1; //set the status so that we know to translate the diagonal element
						}
					}
					//now we need to transpose the elements when the status bit is set
					if(s == 1){
						B[d][d] = temp; //assign the temp now that we're done with that cache section
						s = 0;
					}
				}
			}
		}	
	}

	//61x67 matrix
	if(M == 61 && N == 67){
	blocksize = 20; //after experimenting with different block sizes, 20  works best to reduce misses (which is still a lot)

		for(blockrow = 0; blockrow < N; blockrow += blocksize){
			for(blockcolumn = 0; blockcolumn < M; blockcolumn += blocksize){
				for(i = blockrow; i < blockrow + blocksize; i++){
					for(j = blockcolumn; j < blockcolumn + blocksize; ++j){
						if(i > 66 || j > 60){ //this row and column are already transposed
							continue; //break out of this loop iteration
						}
						else{
							B[j][i] = A[i][j]; //if it is not a diagonal, just transpose that thing
						}
					}
				}
			}
		}	
	}




//end funtion

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

