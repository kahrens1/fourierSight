#ifndef COMPLEX_H 
#define COMPLEX_H

typedef struct{

    float re; 
    float im;

} complex_t;

complex_t complex_add(complex_t a, complex_t b);
complex_t complex_sub(complex_t a, complex_t b);
complex_t complex_mult(complex_t a, complex_t b);


#endif