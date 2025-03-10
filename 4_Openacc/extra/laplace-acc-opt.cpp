#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "timer.hpp"

#define WIDTH      1024
#define HEIGHT     1024
#define TEMP_TOLERANCE 0.01

double temp[HEIGHT+2][WIDTH+2];
double temp_prev[HEIGHT+2][WIDTH+2];

void initialize();

void track_progress(int iter);

int main(int argc, char *argv[]) {
    int i, j;
    int itr = 0;
    double worst_dt = 100;
    long long start, stop;


    #pragma acc data create(temp_prev,temp)
    {
        initialize();

        start = wall_clock_time();

        while ( worst_dt > TEMP_TOLERANCE ) {

            worst_dt = 0.0;

            if (itr % 2) {
                #pragma acc parallel loop collapse(2) reduction(max:worst_dt)
                for(i = 1; i <= HEIGHT; i++) {
                    for(j = 1; j <= WIDTH; j++) {
                        temp[i][j] = 0.25 * (temp_prev[i+1][j] + temp_prev[i-1][j] + temp_prev[i][j+1] + temp_prev[i][j-1]);
                        worst_dt = fmax(fabs(temp[i][j] - temp_prev[i][j]), worst_dt);
                    }
                } 
            } else {
                #pragma acc parallel loop collapse(2) reduction(max:worst_dt)
                for(i = 1; i <= HEIGHT; i++) {
                    for(j = 1; j <= WIDTH; j++) {
                        temp_prev[i][j] = 0.25 * (temp[i+1][j] + temp[i-1][j] + temp[i][j+1] + temp[i][j-1]);
                        worst_dt = fmax(fabs(temp[i][j] - temp_prev[i][j]), worst_dt);
                    }
                } 
            }
            
            if((itr % 100) == 0) {
               #pragma acc update host(temp[HEIGHT:1])
               track_progress(itr);
            }

            itr++;
        }
        
        stop = wall_clock_time();
    }
    

    printf("\nMax error at iteration %d was %f\n", itr-1, worst_dt);
    printf("Total time was %1.6f ms.\n", (float)(stop - start) / 1000000);
}

void initialize(){

    int i,j;

    #pragma acc parallel present(temp_prev,temp)
    {
        #pragma acc loop collapse(2)
        for(i = 1; i <= HEIGHT; i++){
            for (j = 1; j <= WIDTH; j++){
                temp_prev[i][j] = 0.0;
            }
        }

        #pragma acc loop
        for(i = 0; i <= HEIGHT+1; i++) {
            temp_prev[i][0]       = temp[i][0]       = 0.0;
            temp_prev[i][WIDTH+1] = temp[i][WIDTH+1] = (100.0/HEIGHT)*i;
        }

        #pragma acc loop
        for(j = 1; j <= WIDTH; j++) {
            temp_prev[0][j]        = temp[0][j]        = 0.0;
            temp_prev[HEIGHT+1][j] = temp[HEIGHT+1][j] = (100.0/WIDTH)*j;
        }
    }
}

void track_progress(int itr) {
    int i;
    printf("---------- Iteration number: %d ------------\n",
            itr);
    for(i = 0; i <= WIDTH; i += (WIDTH/8)) {
        printf("[%d,%d]: %5.2f  ", i, HEIGHT, temp[HEIGHT][i]);
    }
    printf("\n");
}
