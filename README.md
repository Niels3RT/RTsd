# RTsd
FPV race timing system.

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

Please be aware that the fpga firmware is only provided as binary and NOT under GPL-3.0 license, see /fpga/ directory for info.

# Parts List

| part info | nr |
|------|-------------|
| QM Tech board Spartan6 XC6SLX16 SDRAM (MT48LC16M16A2), NOT DDR3! Around $20 on AliExpress ![QM SP6](pics/vlcsnap-2020-09-14-17h57_smaller.jpg?raw=true "QM SP6") | 1x |
| Xilinx USB plattorm cable (cheap chinese copy does the trick) | 1x |
| ESP32_WROVER-IB module | 1x |
| u-FL to RP-SMA pigtail, most 2,4GHz WiFI Antennas are RP-SMA | 1x |
| 2,4GHz WiFi antenna, see pigtail connector | 1x | 
| USB FTDI-Adapter for programming the ESP32 | 1x |
| 2x7 pin socket (1x7 will do too) | 1x |
| DCDC step down converter | 2x |
| MCP3201 SOP-8 buy a few more on AliExpress, i have seen DOA ones | 5x |
| Micro SD-Card socket, look at picture, there are different layouts | 1x |
| Micro-SD Card 128mb/32GB should work | 1x |
| Pin header | 143x |
| 1206 100n (1uF should do too) ceramic capacitor | 10x |
| 1210 1,0uF/min 35V ceramic capacitor | 10x |
| 1206 330 Ohm resistor | 1x |
| 1206 10 kOhm resistor | 1x |
| SMD capcitor 470u min 6V ca. 6.Xmm diameter | 1x |
| 1206 led (choose your color, might be brighter or a little darker) | 1x |
| 32x2 stackable long pin socket/header | 2x |
| a few cm silicone cable AWG20(ish) and a power connector of your choice | 1x |

# Videos
Some (rather ugly) videos recorded during development, top one is the oldest.  
Klick screenshots to watch on youtube:

1. Running a heat and switching control device while flying, result viewer on right side.
[![Watch the video](pics/vlcsnap-2020-09-14-17h56_smaller.png?raw=true)](https://www.youtube.com/watch?v=o1VrKWGbu_8)

2. Testing race control, exceptions, switching devices (well, browser..), race control on right side.
[![Watch the video](pics/vlcsnap-2020-09-14-17h57_smaller.png?raw=true)](https://www.youtube.com/watch?v=Kj8n9SVtbG0)

3. Another race control test, switching heats.
[![Watch the video](pics/vlcsnap-2020-09-14-18h13_smaller.png?raw=true)](https://www.youtube.com/watch?v=IwP20b6x6Jg)


# ATTENTION
This repository as well as the whole project are still works in progress.

There is not even all information in here (yet) needed to build the device, let alone using it.

So please be patient :)