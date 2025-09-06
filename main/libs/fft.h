#ifndef FFT_H 
#define FFT_H
#include "complex.h"
#include <stdint.h> 
#include <stdbool.h>


#define FFT_USE_BIT_REV_LUT 
#define FFT_USE_TFS_LUT

#define FFT_USE_1024_POINT 
#define FFT_USE_512_POINT 
#define FFT_USE_256_POINT 
#define FFT_USE_128_POINT 
#define FFT_USE_64_POINT 
#define FFT_USE_32_POINT 


typedef struct {

    uint16_t numPoints;
    complex_t *tfs;
    uint16_t *bitRevTable;

} fft_instance_t;


_Bool fft_init(fft_instance_t *fft_init, uint16_t numPoints);
void fft_compute(fft_instance_t *fft_init, complex_t *data);


#endif