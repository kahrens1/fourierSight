#include <stdio.h>
// #include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// #include "esp_chip_info.h"
// #include "esp_system.h"
#include "fft.h"
#include "sph0645.h"


#define SAMPLE_RATE 16000
#define UART_BAUD_RATE 921600
#define FFT_SIZE 1024
#define BUFF_SIZE_SAMPS FFT_SIZE 
#define BUFF_SIZE_BYTES BUFF_SIZE_SAMPS*4

//Perpiphreal Handles
static i2s_chan_handle_t rx_chan;
static fft_instance_t fft;

// Queue Handles 
static QueueHandle_t raw_samps_queue; 
static QueueHandle_t fft_data_queue;


// Task Handles
static TaskHandle_t read_i2s_task_handle;
static TaskHandle_t fft_task_handle; 
static TaskHandle_t uart_task_handle;


static uint64_t dropped_i2s_frames = 0;
static uint64_t dropped_fft_frames = 0; 

// Tasks: 
//     -Read in audio data into a Queue
//     -Compute Queue
//     -Package Data and send to host via UART (how do we want to package FFT data)
//          -Host must be able to reliably detect the start and end of the FFT data frame



static void task_i2s_read(void *args){

    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));
    static size_t bytes_copied = 0;

    while(1){
        
        uint32_t *samp_buf = (uint32_t *)calloc(1,BUFF_SIZE_SAMPS*sizeof(uint32_t)); //Note: Changed from calloc (think this should be okay)
        assert(samp_buf != NULL);

        if (i2s_channel_read(rx_chan, samp_buf, BUFF_SIZE_BYTES, &bytes_copied, portMAX_DELAY) == ESP_OK) { // i2s_channel_read: Returns ESP_OK when number of bytes (BUFF_SIZE_SAMPS) has been copied into the buffer / timeout (1000 TICKS)
        
            sph0645_cook_data(samp_buf, BUFF_SIZE_SAMPS);

            if (xQueueSend(raw_samps_queue,&samp_buf,0) != pdTRUE){
                dropped_i2s_frames += 1;
            }  
        } 
        else {
            free(samp_buf);
            samp_buf = NULL;
            printf("Read Task: i2s read failed\n");
        }

    }
    
    vTaskDelete(NULL);
    return;
}

static void task_fft(void *args){ 

    static uint32_t *samp_buf_ptr;
  
    while(1){
        
        xQueueReceive(raw_samps_queue,&samp_buf_ptr,portMAX_DELAY);
        complex_t *fft_res = (complex_t *)malloc(FFT_SIZE*sizeof(complex_t)); // TODO: Fix so this can be done in place (remove this allocation)
        convert_to_complex(samp_buf_ptr,fft_res,FFT_SIZE);
        free(samp_buf_ptr);
        samp_buf_ptr = NULL;
        fft_compute(&fft,fft_res);
        
        if (xQueueSend(fft_data_queue,&fft_res,0) != pdTRUE){
            dropped_fft_frames += 1; 
        }
 

    }
    


    vTaskDelete(NULL)
    return;
}

static void task_uart_send_fft_data(void *args){ 

    while(1){

    }
    
    vTaskDelete(NULL)
    return;

}



void app_main(void){

    configure_sph0645(&rx_chan,SAMPLE_RATE); 
    configure_uart(UART_BAUD_RATE); 
    assert(fft_init(&fft,FFT_SIZE));
    raw_samps_queue = xQueueCreate(SAMPS_QUEUE_SIZE,sizeof(uint32_t *));
    fft_data_queue = xQueueCreate(FFT_QUEUE_SIZE,sizeof(compelx_t *));

    
}
