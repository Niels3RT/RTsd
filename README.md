# RTsd
FPV race timing system.

![picture of device](pics/RTsd_Geruest_small.jpg?raw=true "RTsd device")

# ATTENTION
This repository as well as the whole project are still works in progress and i'm only human, so expect mistakes and errors.

Please be patient :)

# Features
-4/8 channels  
-around 40kHz sampling rate per channel  
-all channels are sampled simultaneously, at the same clock edges  
-all sampling, timing, peak detection etc. are done in an FPGA (right now a Xilinx Spartan6 XC6SLX16 or Artix7 XC7A35T)  
-WiFi, HTTP server and race housekeeping are done by a ESP32 (ESP32-WROVER-IB module)  
-WiFi works as Access Point or connects to existing network  
-both communicate via an SPI interface with each other  
-ui is in javascript, so smartphone, tablet, computer can be used to control racing  
-a result viewer (html/javascript) for smart tv etc. is included  
-all race/configuration data is saved to sd card, device dependend data (rx module calibration) to esp nv storage  
-as long as the sd card is intact, all other devices can be changed during a race (think of water, fire)  
-all rx5808 modules (i have seen..) are supported (rssi to 1.xV and 3.xV)  
-rssi data is saved and reprocessed in realtime whenever results are requested  
-integrated power supply, just connect lipo (i used 12V/3s for development, 2s/4s *should* work too but untested)  
-simple, easy to use ui  

Please be aware that the fpga firmware is only provided as binary and NOT under GPL-3.0 license, see /fpga/ directory for info.


# Videos
Some (rather ugly and old) videos recorded during development, top one is the oldest.  
Klick screenshots to watch on youtube:

1. Running a heat and switching control device while flying, result viewer on right side.
[![Watch the video](pics/vlcsnap-2020-09-14-17h56_smaller.png?raw=true)](https://www.youtube.com/watch?v=o1VrKWGbu_8)

2. Testing race control, exceptions, switching devices (well, browser..), race control on right side.
[![Watch the video](pics/vlcsnap-2020-09-14-17h57_smaller.png?raw=true)](https://www.youtube.com/watch?v=Kj8n9SVtbG0)

3. Another race control test, switching heats.
[![Watch the video](pics/vlcsnap-2020-09-14-18h13_smaller.png?raw=true)](https://www.youtube.com/watch?v=IwP20b6x6Jg)


# Parts List

Note: Only 1 FPGA board is needed, Spartan6 is cheaper but completely full (4chn only), the Artix7 one costs more but has space for future addons (4 or 8chn).

| part info | 4chn | 8chn |
|------|-------------|----|
| QM Tech board Spartan6 XC6SLX16 SDRAM (MT48LC16M16A2), NOT DDR3! Around $20 on AliExpress<br>![QM SP6](pics/QM-Tech_SP6_XC6SLX16_small.png?raw=true "QM SP6") | 1x | xxx |
| QM Tech board Artix7 XC7A35T DDR3, [Here](https://de.aliexpress.com/item/1000006630084.html)<br>![QM XC7](pics/QM-Tech_Artix7_XC7A35T_small.png?raw=true "QM XC7") | 1x | 1x |
| 0.96" OLED, 7pin spi, optional, only usable on Artix7 board<br>![oled](pics/oled_small.png?raw=true "oled") | 1x | 1x |
| Xilinx USB plattorm cable (cheap chinese copy does the trick) | 1x | 1x |
| rx5808 module, modified for SPI | 4x | 8x |
| ESP32_WROVER-IB module | 1x | 1x |
| u-FL to RP-SMA pigtail, most 2,4GHz WiFI Antennas are RP-SMA | 1x | 1x |
| 2,4GHz WiFi antenna, see pigtail connector | 1x | 1x |
| USB FTDI-Adapter for programming the ESP32 (pictured with pins removed and prepared to fit pin header on pcb)<br>![FTDI](pics/FTDI_small.png?raw=true "FTDI") | 1x | 1x |
| 2x7 pin socket (1x7 will do too) | 1x | 1x |
| DCDC step down converter<br>![DCDC](pics/DCDC_small.png?raw=true "DCDC") | 2x | 3x |
| MCP3201 SOP-8, buy a few more on AliExpress, i have seen DOA ones | 5x | 9x |
| Micro SD-Card socket, look at picture, there are different layouts<br>![FTDI](pics/sd_socket_small.png?raw=true "FTDI") | 1x | 1x |
| Micro-SD Card 32mb/32GB should work (32GB might be enough for a couple of seasons) | 1x | 1x |
| Pin header | 143x | 143x |
| 1206 100n (1uF should do too) ceramic capacitor | 10x | 18x |
| 1210 1,0uF/min 35V ceramic capacitor | 4x | 6x |
| 1206 330 Ohm resistor | 1x | 2x |
| 1206 10 kOhm resistor | 1x | 1x |
| SMD capacitor 470u min 6V ca. 6.Xmm diameter | 1x | 1x |
| 1206 led (choose your color, might be brighter or a little darker) | 1x | 2x |
| 32x2 stackable long pin socket/header<br>![FTDI](pics/long_pin_socket_header_small.png?raw=true "FTDI") | 2x | 6x |
| a few cm silicone cable AWG20(ish) and a power connector of your choice | 1x | 2x |
| some cardboard and self-adhesive copper tape for shielding | 1x | 1x |


# Programming the Xilinx Spartan6 board

This is by far the saddest part of the whole project. Writing the firmware itself is pretty straight forward and a matter of a few clicks. But obtaining the
the tools needed from Xilinx and getting them to run on Windows 10 is not.  

[Here](https://www.youtube.com/watch?v=VMEIPCjqinA) is a nice youtube video on what to download and how to install. To get the download you have to register with Xilinx and
download a couple of GB. In theory a WebPack license will suffice and you only need the Lab Tools installed for the Impact tool.  

! More details will follow, for now just a short walkthrough on where to click when.

In Impact start a new project, let it handle a project file automagically, configure devices using Boundary-Scan, don't assign configuration files right now.  
Right click in the main window and initialize chain.  
Right click on the Xilinx chip and add SPI/BPI flash.  
Choose provided MCS file, select SPI PROM and M25P80, leave data width at 1.  
Left click on the Flash chip above the Xilinx one.  
In the lower left half of the window, double click on program, leave all options as they are.  
Done, one led should be blinking once per second and one led should slowly light up/get darker.  


# Programming the Xilinx Artix7 board

To program the Artix7 you will need the Xilinx Vivado IDE (or its lab tools..), more info on that will follow. The platform cable is the same as for the Spartan6.


# Programming the ESP32

1. Install Python and esptool.  
Windows users see [here](https://cyberblogspot.com/how-to-install-esptool-on-windows-10/).  
On Mac and Linux it *should* work the same way, install Python then esptool (pip?).

Write binaries to the ESP32: Just connect the ftdi to the header on the pcb, change com port to your needs and run command to start flashing.  

![ESP32 FTDI connected](pics/esp_ftdi_small.png?raw=true "ESP32 FTDI connected")

```
esptool -p com6 -b 460800 --before=default_reset --after=hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size 2MB 0x8000 partition-table.bin 0x1000 bootloader.bin 0x10000 trackerAP.bin
```


# Preparing the micro SD-Card

Format card with a FAT derivative, copy the contents of /sd_card/ to the card. The "sd_card_root_here.txt" should be in the root of the filesystem.  

Copy the contents of /html/ to the card /RTsd/http  

Look at the .txt files in RTsd directory and adapt to your preferences.  

Go racing :)


# RTclient

RTclient is a small ESP32_WROVER-IB based pcb. Its purpose is to (cheaply) connect to the system by WiFi and provide racedata to anything you might want to use.
Think of displays, input devices, barcode scanner,...  
~~The first pcbs are badly flawed, gerber files for usable ones will be published later.~~
See gerber directory for RTclient pcb.

![RTclient](pics/rtclient_small.png?raw=true "[RTclient]")

Use case: show pilot names on MultiRX (racedata too, later).  
![RTclient_MultiRX](pics/RTclient_multirx_small.png?raw=true "[RTclient_MultiRX]")