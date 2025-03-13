/**
 *
 * Matrix Multiplication - OpenACC
 *
 * TKR
 *
 **/
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N 102400

int main(int argc, char** argv)
{
    auto type = acc_get_device_type();
    std::string type_s;
    switch (type)
    {
    case acc_device_nvidia:
        type_s = "an Nvidia GPU";
        break;
    case acc_device_host:
        type_s = "the host CPU node";
        break;
    default:
        sprintf(type_s.data(), "an Unknown device (%d)",  type);
    }
    int type_n = acc_get_num_devices(type);
    int type_i = acc_get_device_num(type);

    printf("Hello world!\nDevice is %s, idx %d of %d\n",
        type_s.c_str(), type_i, type_n);
    double v[N];

    for (int i=0; i<N; i++) {
        v[i] = (double)i;
    }
    
    #pragma acc parallel
    {
        #pragma acc loop
        for(int i=0; i<N/2; i++){
            v[i] = sin(v[i]) * 1.2;
        }
        
        #pragma acc loop
        for(int i=N/2; i<N; i++){
            v[i] = cos(v[i]) * 0.4;
        }
    }

    double sum = 0.0;
    for (int i = 0; i < N; i++) {
        sum += v[i];
    }
    printf("Reduction sum: %18.16f\n", sum);
    
    return 0;
}
