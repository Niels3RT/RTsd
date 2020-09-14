# RTsd
FPV racing timing system.

![picture of device](pics/RTsd_Geruest_small.jpg?raw=true "RTsd device")

# Features
-4 channels  
-around 40kHz sampling rate per channel  
-all channels are sampled simultaneously, at the same clock edges  
-all sampling, timing, peak detection etc. are done in an FPGA (right now a Xilinx Spartan6 XC6SLX16)  
-WiFi, HTTP server and race houskeeping are done by a ESP32 (ESP32-WROVER-IB module)  
-both communicate via an SPI interface with each other  
-ui is in javascript, so smartphone, tablet, computer can be used to control racing  
-all race data is saved to sd card, device dependend data (rx module calibration) to esp nv storage  
-as long as the sd card is intact, all other devices can be changed during a race (think of water, fire)  
-all rx5808 modules (i have seen..) are supported (rssi to 1.xV and 3.xV)  
-rssi data is saved and reprocessed in realtime whenever results are requested  
-integrated power supply, just connect lipo  
-simple, easy to use ui  

# Videos
Some (rather ugly) videos recorded during development.  
Klick screenshots to view on youtube:

1. Running a heat and switching control device while flying, result viewer on right side.
[![Watch the video](pics/vlcsnap-2020-09-14-17h56_small.png?raw=true)](https://https://www.youtube.com/watch?v=o1VrKWGbu_8)

1. Testing race control, exceptions, switching devices (well, browser..), race control on right side.
[![Watch the video](pics/vlcsnap-2020-09-14-17h56_small.png?raw=true)](https://https://www.youtube.com/watch?v=Kj8n9SVtbG0)

1. Running a heat and switching control device while flying.
[![Watch the video](pics/vlcsnap-2020-09-14-17h56_small.png?raw=true)](https://https://www.youtube.com/watch?v=o1VrKWGbu_8)


# ATTENTION
This repository as well as the whole project are still works in progress.

There is not even all information in here (yet) needed to build the device yet, let alone using it.

So please be patient :)