import math 
import numpy as np 
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt 

Fs = 40e3
N = 1024

def gen_dt_sin(f,Fs,N):
    Omega = 2*np.pi*f / Fs 
    t = np.arange(0,N/Fs,1/Fs)
    n = np.arange(N)
    x = np.sin(Omega*n)
    return x,t 

def fft_plot(x,Fs,N_fft):
    #X = np.fft.fftshift(np.fft.fft(x))
    X = np.fft.fft(x)
    #W = np.linspace(-np.pi,np.pi,N_fft, endpoint=False)
    W = np.linspace(0,2*np.pi,N_fft, endpoint=False)
    #plt.plot(W/np.pi,20*np.log10(np.abs(X)), '.-')
    plt.plot(W/np.pi,(np.abs(X)) / np.max(np.abs(X)), '.-') #plot without FFT shifting to match ESP implementation 
    plt.plot()
    plt.xlabel(r"$ \Omega/\pi (rad/sample)$")
    plt.ylabel(r"$|X(\Omega) in dB|$")
    return X,W

def gen_samp_array(x,N):
        filename = "samples.h"
        #Convert absolute to ADC values 
        adc_bits = 12 
        adc_max = 2**(adc_bits) - 1
        x_adc = ((adc_max/2) + (adc_max/2)*x).astype(int)
        with open(filename,"w") as f:
            f.write(f"complex_t samps[{int(N)}] = ")
            f.write("{\n")
            for i in range(x.size):
                f.write(f"{{ {x_adc[i]}, 0 }}")
                if i != x.size - 1:
                    f.write(",\n")
            f.write("\n};")
        return x_adc 
             



x,t1 = gen_dt_sin(3e3,Fs,N)
y,t2 = gen_dt_sin(10e3,Fs,N)
z = x + y
#X,W = fft_plot(z,Fs,N,hold=False) #theoretical FFT 

plt.figure()
x_py = gen_samp_array(z,z.size)
X_py, W_py = fft_plot(x_py,Fs,N)


esp_data = np.loadtxt("esp_fft.csv", delimiter = ",")
esp_fft = esp_data[:,0] + 1j*esp_data[:,1]
esp_fft_conj = np.conj(esp_fft)
plt.plot(W_py/np.pi, np.abs(esp_fft) / np.max(np.abs(esp_fft)))

plt.figure()
plt.plot(W_py/np.pi, np.angle(X_py ))
plt.plot(W_py/np.pi, np.angle(esp_fft))

plt.show()

# mag_diff = np.abs (np.abs(X_esp) - np.abs(X_py))
# plt.figure()
# plt.plot(range(N),mag_diff)
# plt.show(block=True)



