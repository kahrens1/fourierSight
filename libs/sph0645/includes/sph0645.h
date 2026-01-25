#ifndef SPH0645_DRIVER 
#define SPH0645_DRIVER
#include <stdint.h>
#include "driver/i2s_std.h"
#include "complex.h"

void configure_sph0645(i2s_chan_handle_t *rx_chan,const uint16_t f_s);
void sph0645_cook_data(uint32_t *samps,size_t num_samps);
void convert_to_complex(uint32_t *samps,complex_t *complex_samps,size_t num_samps);

#endif