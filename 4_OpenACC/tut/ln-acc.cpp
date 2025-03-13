#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "timer.hpp"

#ifdef _OPENACC
#include <openacc.h>
#endif

// Tune this
    #define G 2

#define N 524288
#define X G*16
#define Y N/X

int main(int argc, char** argv)
{
    double** v;
    v = (double**)malloc(sizeof(double*) * X);    
    for (int i = 0; i < X; i++)
        v[i] = (double*)malloc(sizeof(double) * Y);

    auto start = wall_clock_time();
    
    #pragma acc parallel num_gangs(G)
    #pragma acc loop gang worker
    for(int i=0; i<X; i++)
    {
        #pragma acc loop vector
        for(int j=0; j<Y; j++)
        {
            v[i][j] = atan(i*Y+j);
        }
    }

    auto end = wall_clock_time();

    double sum = 0.0;
    for(int i=0; i<X; i++){
        for(int j=0; j<Y; j++){
            sum += v[i][j];
        }
    }

    for (int i = 0; i < X; i++) {
        free(v[i]);
    }
    free(v); 

    printf("Reduction sum: %18.16f\n", sum);
    printf("Took %1.2f ms\n", ((float)(end - start)) / 1000000);
    
    return 0;
}
