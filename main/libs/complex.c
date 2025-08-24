#include"complex.h"


complex_t complex_add(complex_t a, complex_t b){
    complex_t retval; 
    retval.re = a.re + b.re;
    retval.im = a.im + b.im;
    return retval;
}

complex_t complex_sub(complex_t a, complex_t b){
    complex_t retval; 
    retval.re = a.re - b.re;
    retval.im = a.im - b.im;
    return retval;
}

complex_t complex_mult(complex_t a, complex_t b){
    complex_t retval;
    retval.re = (a.re*b.re) - (a.im*b.im);
    retval.im = (a.re*b.im) + (a.im*b.re);
    return retval;
}