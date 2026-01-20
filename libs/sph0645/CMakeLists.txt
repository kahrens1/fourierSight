idf_component_register(
    SRCS "sph0645.c" 
    INCLUDE_DIRS "includes" 
    REQUIRES
        esp_driver_i2s esp_driver_gpio 
)
