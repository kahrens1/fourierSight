#ifndef FFT_H 
#define FFT_H
#include "complex.h"
#include <stdint.h> 
#include <stdbool.h>

typedef struct {
    uint16_t numPoints;
    complex_t *tfs;
    uint16_t *bitRevTable;
    _Bool bBitRevFlag;
} fft_instance;


void fft_init(fft_instance *fft_init, uint16_t numPoints, _Bool bRevFlag);
void fft_compute(fft_instance *fft_init, complex_t *data);


#endif