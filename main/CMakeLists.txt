idf_component_register(
    SRCS "main.c" "libs/fft.c"
    PRIV_REQUIRES spi_flash
    INCLUDE_DIRS "." "libs"
)
