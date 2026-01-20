idf_component_register(
    SRCS "uart_utils.c" 
    INCLUDE_DIRS "includes" 
    REQUIRES
        esp_driver_uart
)
