#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "fft.h"
#include"samples.h"


void app_main(void)
{
    fft_instance_t fft;
    _Bool bfft; 
    bfft = fft_init(&fft,32);
    if (bfft)
        fft_compute(&fft,samps);
    
    for(uint16_t i = 0; i < fft.numPoints; i++){
        printf("%f,%f\n",samps[i].re, samps[i].im);
    }


}
