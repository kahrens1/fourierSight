import math 
import numpy as np 
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt 

fft_sizes = [32,64,128,256,512,1024]
Fs = 44.1e3
N = 512


def gen_dt_sin(f,Fs,N):
    Omega = 2*np.pi*f / Fs 
    t = np.arange(0,N/Fs,1/Fs)
    n = np.arange(N)
    x = np.sin(Omega*n)
    return x,t 

def fft_plot(x,Fs,N_fft):
    X = np.fft.fftshift(np.fft.fft(x))
    W = np.linspace(-np.pi,np.pi,N_fft, endpoint=False)
    plt.plot(W/np.pi,np.abs(X), '.-')
    plt.show()
    return X,W

x,t1 = gen_dt_sin(15e3,Fs,N)
y,t2 = gen_dt_sin(10e3,Fs,N)
z = x + y
X,W = fft_plot(z,Fs,N)