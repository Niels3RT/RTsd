# RTsd
FPV racing timing system.

![picture of device](pics/RTsd_Geruest_small.jpg?raw=true "RTsd device")

# Features
-4 channels  
-around 40kHz sampling rate per channel  
-all channels are sampled simultaneously, at the same clock edges  
-all sampling, timing, peak detection etc. are done in an FPGA (right now a Xilinx Spartan6 XC6SLX16)  
-WiFi, HTTP server and race houskeeping are done by a EPS32 (ESP32-WROVER-IB module)  
-both communicate via an SPI interface with each other  

# ATTENTION
This repository as well as the whole project are still works in progress.

There is not even all information in here needed to build the device, let alone using it.

So please be patient :)