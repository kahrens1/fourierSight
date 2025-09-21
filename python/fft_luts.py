import math 
import numpy as np 


fft_sizes = [32,64,128,256,512,1024]

def generate_bit_rev(i,N_bits):
    rev = 0
    for _ in range(N_bits):
        rev = (rev << 1) | (i & 1)
        i  = (i >> 1) 

    return rev

#TODO: Collapse into single header file/loop 

for fft_size in fft_sizes:
    filename = f"bit_rev_lut_{fft_size}.h"
    N_bits = int(math.log2(fft_size))
        
    with open(filename,"w") as f: 
        if fft_size <= 256:
            f.write(f"static const uint8_t bitRev{fft_size}[{fft_size}] = ")
            f.write("{\n")
        elif fft_size > 256:
            f.write(f"static const uint16_t bitRev{fft_size}[{fft_size}] = ")
            f.write("{\n")

        for i in range(fft_size):
            bit_rev = generate_bit_rev(i,N_bits)
            f.write(f"{bit_rev}")
            if i != fft_size - 1:
                f.write(",")
            if not ((i+1) % 4):
                f.write("\n")
            
        f.write("};")

#Code to generate TFs:

for fft_size in fft_sizes: 
    filename = f"tf_lut{fft_size}.h"
    tf_current = np.array([1,0],dtype=float)
    tf_next = np.zeros(2, dtype=float)
    tf = np.zeros(2, dtype=float)
    d_theta = float(2*np.pi / fft_size)
    print(f"d_theta = {d_theta}")
    d_cos = float(np.cos(d_theta))
    print(d_cos)
    d_sin = float(np.sin(d_theta))
    print(d_sin)
    theta = float(0)
    
    with open(filename,"w") as f: 
        f.write(f"static const complex_t tfs{fft_size}[{int(fft_size/2)}] = ")
        f.write("{\n")
       
        for i in range(int(fft_size/2)): 
            tf[0] = np.cos(theta)
            tf[1] = -np.sin(theta)
            theta = float(theta + d_theta)

            f.write(f"{{ {tf[0]}f, {tf[1]}f }}")
           
            if i != (fft_size/2) - 1:
                f.write(",\n")
           
            # tf_next[0] = d_cos*tf_current[0] - d_sin*tf_current[1]
            # tf_next[1] = np.conj(d_cos*tf_current[1] + d_sin*tf_current[0])
            # tf_current = tf_next 

        f.write("\n};")
