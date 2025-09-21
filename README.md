# Building the Program (from WSL setup)
    Configure WSL USB Passthrough: 
        -Run *usbipd list* from Powershell 
        -Find BUSID of ESP32. ESP will show up on a COM port as USB to UART bridge 
        -Run *usbipd attach --busid <BUSID> --wsl Ubuntu-22.04*
# Setup Enviroment in WSL
    -Check if passthrough has been configured correctly. Run *ls /dev/ttyUSB* * 
    -From ~/esp-idf, run *source ./export.sh* 
    -Build the program: Run *idf.py build* from fourierSight directory
    -Flash the ESP: Run *idf.py -p /dev/ttyUSBX* 
    -Open serial monitor: Run *idf.py -p /dev/ttyUSBX monitor*cd


