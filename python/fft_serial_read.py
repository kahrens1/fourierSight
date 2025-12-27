import serial 
import numpy as np 

def fft_serial_read(port,baud,N): 
    ser = serial.Serial(port,baud,timeout=1)

    for _ in range(N):
        line = ser.readline().decode().strip()  # Read a line
        if line:
            parts = line.split(',')
            if len(parts) == 2:
                fft_data.append([float(parts[0]), float(parts[1])])

    ser.close()

    fft_data = np.array(fft_data)
    fft_complex = fft_data[:,0] + 1j*fft_data[:,1]

    # Save to CSV
    np.savetxt("esp_fft.csv", fft_data, delimiter=",")


fft_serial_read('/dev/ttyUSB0',115200,512)