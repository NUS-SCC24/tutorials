#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "timer.hpp"

#define WIDTH      1024
#define HEIGHT     1024
#define TEMP_TOLERANCE 0.01

double plate_a[HEIGHT+2][WIDTH+2];
double plate_b[HEIGHT+2][WIDTH+2];

void initialize();

void track_progress(int iter);

int main(int argc, char *argv[]) {
    int i, j;
    int itr = 0;
    double worst_dt = 100;
    long long start, stop;

    initialize();
    
    double (*temp)[HEIGHT+2] = plate_b, (*temp_prev)[HEIGHT+2] = plate_a;   

    start = wall_clock_time();

    while ( worst_dt > TEMP_TOLERANCE ) {
        
        worst_dt = 0.0;
        
        for(i = 1; i <= HEIGHT; i++) {
            for(j = 1; j <= WIDTH; j++) {
                temp[i][j] = 0.25 * (temp_prev[i+1][j]
                        + temp_prev[i-1][j]
                        + temp_prev[i][j+1]
                        + temp_prev[i][j-1]);
                worst_dt = fmax(fabs(temp_prev[i][j] - temp[i][j]), worst_dt);
            }
        }

        double (*t)[HEIGHT+2] = temp;
        temp = temp_prev;
        temp_prev = t;

        if((itr % 100) == 0) {
            track_progress(itr);
        }

        itr++;
    } 
    stop = wall_clock_time();

    printf("\nMax error at itr %d was %f\n", itr-1, worst_dt);
    printf("Total time was %1.6f ms.\n", (float)(stop - start) / 1000000);
}

void initialize(){
    int i,j;
    double x;
    for(i = 0; i <= HEIGHT+1; i++){
        for (j = 0; j <= WIDTH+1; j++){
            plate_a[i][j] = 0.0;
            plate_b[i][j] = 0.0;
        }
    }
    for(i = 0; i <= HEIGHT+1; i++) {
        x = (100.0/HEIGHT)*i;
        plate_a[i][WIDTH+1] = x;
        plate_b[i][WIDTH+1] = x;
    }
    for(j = 0; j <= WIDTH+1; j++) {
        x = (100.0/WIDTH)*j;
        plate_a[HEIGHT+1][j] = x;
        plate_b[HEIGHT+1][j] = x;
    }
}
void track_progress(int itr) {
    int i;
    printf("---------- Iteration number: %d ------------\n",
            itr);
    for(i = 0; i <= HEIGHT; i += (HEIGHT/8)) {
        printf("[%d,%d]: %5.2f  ", i, HEIGHT, plate_b[i][HEIGHT]);
    }
    printf("\n");
}
