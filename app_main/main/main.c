#include <stdio.h>
// #include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// #include "esp_chip_info.h"
// #include "esp_system.h"
#include "fft.h"
#include "sph0645.h"
#include "uart_utils.h"

// Constants
#define SAMPLE_RATE 16000
#define UART_BAUD_RATE 921600
#define FFT_SIZE 1024
#define BUFF_SIZE_SAMPS FFT_SIZE 
#define BUFF_SIZE_BYTES BUFF_SIZE_SAMPS*4
#define FFT_QUEUE_SIZE 4
#define SAMPS_QUEUE_SIZE 4
const uint32_t sof_bytes = 0xDEADBEEF;

// Task Priorities

#define I2S_TASK_PRIORITY 3
#define FFT_TASK_PRIORITY 2
#define UART_TASK_PRIORITY 1


// Perpiphreal Handles
static i2s_chan_handle_t rx_chan;
static fft_instance_t fft;

// Queue Handles 
static QueueHandle_t raw_samps_queue; 
static QueueHandle_t fft_data_queue;


// Task Handles
static TaskHandle_t i2s_task_handle;
static TaskHandle_t fft_task_handle; 
static TaskHandle_t uart_task_handle;


static uint64_t dropped_i2s_frames = 0;
static uint64_t dropped_fft_frames = 0; 

//TODO: Implement Static Buffer Pool instead of dynamic allocation

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
    static complex_t fft_res[FFT_SIZE];
  
    while(1){
        
        xQueueReceive(raw_samps_queue,&samp_buf_ptr,portMAX_DELAY);
        uint32_t *fft_mags = (uint32_t *)malloc(FFT_SIZE*sizeof(uint32_t)); // TODO: Fix so this can be done in place (remove this allocation)
        convert_to_complex(samp_buf_ptr,fft_res,FFT_SIZE);
        free(samp_buf_ptr);
        samp_buf_ptr = NULL;
        fft_compute(&fft,fft_res);
        squared_magnitude_compute(&fft,fft_mags,fft_res);
        
        if (xQueueSend(fft_data_queue,&fft_mags,0) != pdTRUE){
            free(fft_mags);
            dropped_fft_frames += 1; 
        }

    }
    


    vTaskDelete(NULL);
    return;
}

static void task_uart_send_fft_data(void *args){ 

    static uint32_t *fft_mags_ptr;

    while(1){
        xQueueReceive(fft_data_queue, &fft_mags_ptr ,portMAX_DELAY);
        uart_write_bytes(UART_NUM_0,&sof_bytes,sizeof(uint32_t));
        uart_write_bytes(UART_NUM_0,(const void *)fft_mags_ptr,FFT_SIZE*sizeof(uint32_t)); 
        free(fft_mags_ptr);
    }
    
    vTaskDelete(NULL);
    return;

}



void app_main(void){

    configure_sph0645(rx_chan,SAMPLE_RATE); 
    configure_uart(UART_BAUD_RATE); 
    assert(fft_init(&fft,FFT_SIZE));

    raw_samps_queue = xQueueCreate(SAMPS_QUEUE_SIZE,sizeof(uint32_t *));
    fft_data_queue = xQueueCreate(FFT_QUEUE_SIZE,sizeof(uint32_t *));

    xTaskCreate(task_i2s_read,"i2s", 4096, NULL, I2S_TASK_PRIORITY, &i2s_task_handle);
    xTaskCreate(task_fft,"fft",6144, NULL, FFT_TASK_PRIORITY, &fft_task_handle);
    xTaskCreate(task_uart_send_fft_data,"uart",4096, NULL, UART_TASK_PRIORITY, &uart_task_handle);

}
