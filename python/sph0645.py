import numpy as np
import wave 
import serial
import math 
import matplotlib.pyplot as plt 
import matplotlib.animation as animation
from queue import Queue
from fft_plot import fft_plot
import threading


class sph0645:
    
    READ_BUFFER_SIZE = 4096 #Likely need to increase size 
    Fs = 16000
    WAV_CONVERSION_FACT = ( 32768 / 131072 )
    BYTES_PER_SAMPLE = 4 
    NUM_VALID_BITS = 18
    START_OF_FILE = bytearray(b"\xef\xbe\xad\xde") #UART Sent as little endian
    FFT_SIZE = 1024
    QUEUE_SIZE = 4

    def __init__(self,port="/dev/ttyUSB0",baud=921600,wav_file="test.wav"):
        
        self.ser_port = serial.Serial(port,baud)
        self.wav_file = wav_file 
        self.samps_buf = bytearray()
        self.window = bytearray(4)
        self.start_flag = True
        self.end_flag = False
        self.first_run_f = False 
        self.win_cntr = 0 
        self.raw_samps = []
        self.cooked_samples = []
        self.samps_cntr = 0 
        self.fft_samps_queue = Queue(maxsize=self.QUEUE_SIZE)
        self.dropped_fft_frames = 0
        self.fig, self.ax = plt.subplots()
        
        f = self.init_fft_plot()
        self.fft_line, = self.ax.plot(f,np.zeros(self.FFT_SIZE))

    def read_audio_data(self): 

        print("Press CTRL+C to end Audio Capture")
        while True: 
            try: 
                samps = self.ser_port.read(self.READ_BUFFER_SIZE)
                
                for samp in samps: 
                    self.window.pop(0)
                    self.window.append(samp)

                    if self.start_flag: 
                        self.samps_buf.append(samp)
                        self.win_cntr += 1 
                        if self.win_cntr == self.BYTES_PER_SAMPLE: 
                            self.raw_samps.append(bytes(self.window))
                            self.win_cntr = 0  
                            self.samps_cntr += 1
                    if self.window == self.START_OF_FILE:
                        print("Found Start Audio Data")
                        self.start_flag = True
        
            except KeyboardInterrupt: 
                print("Capture Stopped") 
                break 
        print(f"Captured {len(self.samps_buf)} total Samples")

    def stream_audio_data(self): 
        print("Press CTRL+C to end Streaming")
        

        while True: 
            try: 
                cooked_samps = []
                samps = self.ser_port.read(self.READ_BUFFER_SIZE)
                for samp in samps:
                    self.window.pop(0)
                    self.window.append(samp)
                    self.samps_buf.append(samp)
                    self.win_cntr += 1 
                    if self.win_cntr == self.BYTES_PER_SAMPLE: 
                        cooked_samps.append(self.cook_sample(self.window))
                        self.win_cntr = 0  
                        self.samps_cntr += 1
                
                fft_samps = np.array(cooked_samps)
                fft_samps = fft_samps - np.mean(fft_samps)
                fft_samps =  fft_samps / max(np.abs(fft_samps))

                if not self.fft_samps_queue.full(): #Getting more than 1024 samples
                    self.fft_samps_queue.put(fft_samps)
                else:
                    self.dropped_fft_frames += 1

            except KeyboardInterrupt: 
                print(f"Streaming Stopped, Dropped {self.dropped_fft_frames} Frames")
                break

    
    def init_fft_plot(self): 
    
        f = np.linspace(-self.Fs/2,self.Fs/2,self.FFT_SIZE, endpoint=False)
        self.ax.set_xlim(-self.Fs/2,self.Fs/2)
        self.ax.set_ylim(0,500)
        return f

    def update_fft_plot(self,frame): 
        if not self.fft_samps_queue.empty(): 
            x = self.fft_samps_queue.get() 
            x = x[0:1024]
            X = np.fft.fftshift(np.fft.fft(x))
            self.fft_line.set_ydata(np.abs(X))

    
    def ani_fft_plot(self): 
        ani = animation.FuncAnimation(self.fig,self.update_fft_plot,interval=20,cache_frame_data=False)
        plt.show()


    def cook_samples(self): 

        for samp in self.raw_samps:
            self.cooked_samples.append((int.from_bytes(samp,byteorder='little', signed=True) >> (32 - self.NUM_VALID_BITS) ))

    def cook_sample(self,samp):
        return int.from_bytes(samp,byteorder='little', signed=True) >> (32 - self.NUM_VALID_BITS) 

    def convert_to_wav(self): 
            
        samples = np.array(self.cooked_samples,dtype=float)
        samples = (samples*self.WAV_CONVERSION_FACT).astype(np.int16)


        with wave.open(self.wav_file, "w") as wf:
            wf.setnchannels(1)
            wf.setsampwidth(2)  
            wf.setframerate(self.Fs)
            wf.writeframes(samples.tobytes())

        print(f"WAV written to {self.wav_file}")






if __name__ == "__main__": 
    
    mic = sph0645()

    #Read and convert to .wav
 
    # mic.read_audio_data()
    # mic.cook_samples()
    # mic.convert_to_wav()
    #Take fft of single window see what happens
    #FFT Vis

    threading.Thread(target=mic.stream_audio_data, daemon=True).start()
    mic.ani_fft_plot()










