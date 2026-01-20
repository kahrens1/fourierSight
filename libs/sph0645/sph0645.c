#include"sph0645.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "complex.h"

#define SPH0645_NUM_VALID_BITS 18



void configure_sph0645(i2s_chan_handle_t *rx_chan,const uint16_t f_s){

    i2s_chan_config_t rx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&rx_chan_cfg, NULL, &rx_chan));

    i2s_std_config_t rx_std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(f_s), //BCLK pin speed =  Fs * 32 bits per sample * 1 (we are working with Mono). WS/LRCLK will have speed 16 kHz
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO), 
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,   
            .bclk = GPIO_NUM_22,
            .ws   = GPIO_NUM_23,
            .dout = I2S_GPIO_UNUSED,
            .din  = GPIO_NUM_26,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &rx_std_cfg));
}

void sph0645_cook_data(uint32_t *samps,size_t num_samps){

    int64_t sum_samps;
    int64_t dc_offset;

    for(size_t i = 0; i < num_samps; i++){
        samps[i] = samps[i] >> (32 - SPH0645_NUM_VALID_BITS);
        sum_samps += samps[i];
    }
   
    dc_offset = sum_samps / num_samps;

    for(size_t i = 0; i < num_samps; i++){
        samps[i] -= dc_offset;
    }

    // Need to add peak find ??

}

void convert_to_complex(uint32_t *samps,complex_t *complex_samps,size_t num_samps){ 

    for(size_t i = 0; i < num_samps; i++){
        complex_samps[i].re = samps[i];
        complex_samps[i].im = 0;
    }

}