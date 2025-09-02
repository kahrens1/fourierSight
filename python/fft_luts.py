import math 

fft_sizes = [32,64,128,256,512,1024]
#filename = "fft_luts.txt"

def generate_bit_rev(i,N_bits):
    rev = 0
    for _ in range(N_bits):
        rev = (rev << 1) | (i & 1)
        i  = (i >> 1) 

    return rev

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


#Add Code to generate TFs
