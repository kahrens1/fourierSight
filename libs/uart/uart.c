#include"driver/uart.h"

void configure_uart(const uint32_t baud){ //Note: Hardcoded to UART Channel 0 
    
    uart_config_t uart_config = {
        .baud_rate = baud,
        .data_bits = UART_DATA_8_BITS, 
        .parity = UART_PARITY_DISABLE, 
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, 
        .source_clk = UART_SCLK_APB
    };
    
  
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, 4096, 0, 0, NULL, 0); //Meaning of this?
 

}