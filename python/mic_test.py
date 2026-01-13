#TODO: Write script to consume UART data from ESP and convert to .wav
# Actually Read from the UART instead of using Minicom (was the format i sent bits in the same as what I am parsing as (parity/stop bit ect.))
# Lenght of resulting .wav is not close at all to recording length (I am probably just dropping samples)

import numpy as np
import wave
import struct
import serial 

PORT = "/dev/ttyUSB0"
BAUD = 921600
READ_BUFFER_SIZE = 4096
SAMPLE_RATE = 16000
WAV_CONVERSION_FACT = ( 32768 / 131072 )
WAV_FILE = "test.wav"
BYTES_PER_SAMPLE = 4 
NUM_VALID_BITS = 18
START_OF_FILE = bytearray(b"\xef\xbe\xad\xde") #UART Sent as little endian
END_OF_FILE = bytearray(b"\xe4\xbe\xad\xde") 
samps_buf = bytearray()
window = bytearray(4)
start_flag = False
end_flag = False
win_cntr = 0 
raw_samps = []
samps_cntr = 0 



#TODO: Function to convert to the correct format (24-bit, 2's complement, MSB first), then plot

def cook_sph0645(raw_samps): 
    cooked_samps = []
    for samp in raw_samps:
        cooked_samps.append((int.from_bytes(samp,byteorder='little', signed=True) >> (32 - NUM_VALID_BITS) ))

    print(cooked_samps)
    return cooked_samps

def convert_to_wav(cooked_samps): 
    
    samples = np.array(cooked_samps,dtype=float)
    samples = (samples*WAV_CONVERSION_FACT).astype(np.int16)


    with wave.open(WAV_FILE, "w") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)  
        wf.setframerate(SAMPLE_RATE)
        wf.writeframes(samples.tobytes())

    print(f"WAV written to {WAV_FILE}")



ser = serial.Serial(PORT,BAUD)
print("Opening Serial Port")
print("Press CRTL+C to end Audio Capture")

while True: 
    try: 
        samps = ser.read(READ_BUFFER_SIZE)
        
        for samp in samps: 
            window.pop(0)
            window.append(samp)

            if start_flag: 
                samps_buf.append(samp)
                win_cntr += 1 
                if win_cntr == BYTES_PER_SAMPLE: 
                    raw_samps.append(bytes(window))
                    win_cntr = 0  
                    samps_cntr += 1
                    print(f"Win Counter: {samps_cntr}, Window {window}")

            if window == START_OF_FILE:
                print("Found Start Audio Data")
                start_flag = True
            if window == END_OF_FILE: #Not working with long reads, some sort of alignement issue
                print("Found End of Audio Data")
                end_flag = True
                start_flag = False
                break

    except KeyboardInterrupt: 
        print("Capture Stopped") 
        break

print(f"Captured {len(samps_buf)} total Samples")
print("Cooked Samples")
cooked_samps = cook_sph0645(raw_samps)
convert_to_wav(cooked_samps)



