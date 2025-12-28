
/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "sdkconfig.h"

/* Set 1 to allocate rx & tx channels in duplex mode on a same I2S controller, they will share the BCLK and WS signal
 * Set 0 to allocate rx & tx channels in simplex mode, these two channels will be totally separated,
 * Specifically, due to the hardware limitation, the simplex rx & tx channels can't be registered on the same controllers on ESP32 and ESP32-S2,
 * and ESP32-S2 has only one I2S controller, so it can't allocate two simplex channels */
#define CONFIG_USE_DUPLEX 0
#define EXAMPLE_I2S_DUPLEX_MODE         CONFIG_USE_DUPLEX

#define EXAMPLE_STD_BCLK_IO1        GPIO_NUM_4      // I2S bit clock io number
#define EXAMPLE_STD_WS_IO1          GPIO_NUM_5      // I2S word select io number
#define EXAMPLE_STD_DOUT_IO1        GPIO_NUM_18     // I2S data out io number
#define EXAMPLE_STD_DIN_IO1         GPIO_NUM_19     // I2S data in io number

#if !EXAMPLE_I2S_DUPLEX_MODE
#define EXAMPLE_STD_BCLK_IO2        GPIO_NUM_22     // I2S bit clock io number
#define EXAMPLE_STD_WS_IO2          GPIO_NUM_23     // I2S word select io number
#define EXAMPLE_STD_DOUT_IO2        GPIO_NUM_25     // I2S data out io number
#define EXAMPLE_STD_DIN_IO2         GPIO_NUM_26     // I2S data in io number
#endif

#define EXAMPLE_BUFF_SIZE               2048

static i2s_chan_handle_t                tx_chan;        // I2S tx channel handler
static i2s_chan_handle_t                rx_chan;        // I2S rx channel handler

static void i2s_example_read_task(void *args)
{
    uint8_t *r_buf = (uint8_t *)calloc(1, EXAMPLE_BUFF_SIZE);
    assert(r_buf); // Check if r_buf allocation success
    size_t r_bytes = 0;

    /* Enable the RX channel */
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));

    /* ATTENTION: The print and delay in the read task only for monitoring the data by human,
     * Normally there shouldn't be any delays to ensure a short polling time,
     * Otherwise the dma buffer will overflow and lead to the data lost */
    while (1) {
        /* Read i2s data */
        if (i2s_channel_read(rx_chan, r_buf, EXAMPLE_BUFF_SIZE, &r_bytes, 1000) == ESP_OK) {
            printf("Read Task: i2s read %d bytes\n-----------------------------------\n", r_bytes);
            printf("[0] %x [1] %x [2] %x [3] %x\n[4] %x [5] %x [6] %x [7] %x\n\n",
                   r_buf[0], r_buf[1], r_buf[2], r_buf[3], r_buf[4], r_buf[5], r_buf[6], r_buf[7]);
        } else {
            printf("Read Task: i2s read failed\n");
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    free(r_buf);
    vTaskDelete(NULL);
}



static void i2s_example_init_std_simplex(void)
{

    i2s_chan_config_t rx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&rx_chan_cfg, NULL, &rx_chan));

    i2s_std_config_t rx_std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO), //Problem here?
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,    // some codecs may require mclk signal, this example doesn't need it
            .bclk = EXAMPLE_STD_BCLK_IO2,
            .ws   = EXAMPLE_STD_WS_IO2,
            //.dout = EXAMPLE_STD_DOUT_IO2,
            .dout = I2S_GPIO_UNUSED,
            .din  = EXAMPLE_STD_DIN_IO2,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    /* Default is only receiving left slot in mono mode,
     * update to right here to show how to change the default configuration */
     // rx_std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_RIGHT; 
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &rx_std_cfg));
}

void app_main(void)
{

    i2s_example_init_std_simplex();
    xTaskCreate(i2s_example_read_task, "i2s_example_read_task", 4096, NULL, 5, NULL);
}
