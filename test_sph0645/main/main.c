#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "driver/uart.h"
#include "sdkconfig.h"
#include "driver/i2s_std.h"


#define BYTES_PER_SAMP 4
#define BUFF_SIZE_SAMPS 100
#define BUFF_SIZE_BYTES BYTES_PER_SAMP*BUFF_SIZE_SAMPS
#define MASK_SPH0645_VALID_BITS 0xFFFFC // Data sent in 32 bit packets but only 18 bits of percision
#define SAMPS_QUEUE_SIZE 4
#define SPH0645_NUM_VALID_BITS 18
#define SAMP_NORM_FACT 19

static i2s_chan_handle_t rx_chan;        // I2S rx channel handler
static QueueHandle_t samp_buff_queue;
static TaskHandle_t uart_task_handle;
static TaskHandle_t i2s_task_handle;
static uint8_t dropped_frames = 0;
static _Bool bStartFlag; 
static _Bool bEndFlag = false;
const char *sof = "\nStart of Audio Data\n";
const uint32_t sof_bytes = 0xDEADBEEF;
const char *eof = "\nEnd of Audio Data\n";
const uint32_t eof_bytes = 0xDEADBEE4;

static void sph0645_cook_data(uint32_t *samps,size_t num_samps){

    int32_t *samples = (int32_t *) samps;
    int64_t sum_samps = 0;
    int64_t dc_offset = 0;
    uint32_t max_val = 0; 
    int32_t scale;

    for(size_t i = 0; i < num_samps; i++){
        samples[i] = samples[i] >> (32 - SPH0645_NUM_VALID_BITS);
        sum_samps += samples[i];
    }

    dc_offset = sum_samps / num_samps;
    
    // Subtract DC offset and peak search
    for(size_t i = 0; i < num_samps; i++){
        samples[i] -= dc_offset;

        if(abs(samples[i]) > max_val){
            max_val = samples[i];
        }
    }

    // if(max_val == 0)
    //     return;

    // scale = (1 << 30) / max_val; // Fixed point Q30

    // // Normalize
    // for(size_t i = 0; i < num_samps; i++){
    //     samples[i] = (samples[i]*scale) >> 30;
    // }

}

static void task_i2s_read(void *args)
{


    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));
    static uint16_t task_cntr = 0;
    size_t bytes_copied = 0;

   
    while (1) {

        uint32_t *samps_buff = (uint32_t *)calloc(1,BUFF_SIZE_SAMPS*sizeof(uint32_t));
        assert(samps_buff != NULL);

        if (i2s_channel_read(rx_chan, samps_buff, BUFF_SIZE_BYTES, &bytes_copied, portMAX_DELAY) == ESP_OK) { // i2s_channel_read: Returns ESP_OK when number of bytes (BUFF_SIZE_SAMPS) has been copied into the buffer / timeout (1000 TICKS)
        
            sph0645_cook_data(samps_buff, BUFF_SIZE_SAMPS);

            if (xQueueSend(samp_buff_queue,&samps_buff,0) != pdTRUE){
                free(samps_buff);
                dropped_frames += 1;
            }  
        } 
        else {
            printf("Read Task: i2s read failed\n");
        }

        if (task_cntr == 1)
            bStartFlag = true;

        task_cntr += 1;
    }
    
    vTaskDelete(NULL);
}

static void task_uart_send_i2s_data(void *args){
   
    static uint32_t * samp_buff_ptr;

    while(1){

        xQueueReceive(samp_buff_queue, &samp_buff_ptr ,portMAX_DELAY);
        
        if (bStartFlag){
            // uart_write_bytes(UART_NUM_0,sof,strlen(sof));
            uart_write_bytes(UART_NUM_0,&sof_bytes,sizeof(uint32_t));
            bStartFlag = false;
        }


        uart_write_bytes(UART_NUM_0,
                            (const void *)samp_buff_ptr,
                            BUFF_SIZE_SAMPS * sizeof(uint32_t));
        free(samp_buff_ptr);

        if (bEndFlag && uxQueueMessagesWaiting(samp_buff_queue) == 0){
            uart_write_bytes(UART_NUM_0,&eof_bytes,sizeof(uint32_t));
            break;
        }
    }   

    vTaskDelete(NULL);
    return;

}


static void i2s_example_init_std_simplex(void)
{

    i2s_chan_config_t rx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&rx_chan_cfg, NULL, &rx_chan));

    i2s_std_config_t rx_std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(16000), //Sets Audio Sampling rate to 16 kHz. BCLK pin speed = 16 kHz * 32 bits per sample * 1 (we are working with Mono). WS/LRCLK will have speed 16 kHz
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

static void configure_uart( void ){
    
    uart_config_t uart_config = {
        .baud_rate = 921600,
        .data_bits = UART_DATA_8_BITS, 
        .parity = UART_PARITY_DISABLE, 
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, 
        .source_clk = UART_SCLK_APB
    };
    
  
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, 4096, 0, 0, NULL, 0);
 

}

void app_main(void)
{

    i2s_example_init_std_simplex();
    configure_uart();

    samp_buff_queue = xQueueCreate(SAMPS_QUEUE_SIZE,sizeof(uint32_t *));
    assert(samp_buff_queue != NULL);
    xTaskCreate(task_i2s_read, "task_i2s_read", 2048, NULL,2,&i2s_task_handle); 
    xTaskCreate(task_uart_send_i2s_data, "task_uart_send_i2s_data", 2048, NULL, 1, &uart_task_handle); // Task should consume items faster then they are added

    while(1){
        printf("Dropped Frames %d",dropped_frames);
        vTaskDelay(portMAX_DELAY);
    }


}
