# ESP-Web2SD-Mgr
Simple Web server to manage your SD card files

# How to use
* Download this project and open in VsCode/PlatformIO
* Choose your Env, or add the tft configurations and SD card pins
* burn or build

After burned, navigation will use only one button, configured in platformio.ini

To configure it to connect into your netwprk, after the first boot, it will create  file called: ESPWeb2SD.txt
Change the first line string "SSID_of_my_network" with your SSID Network and "Password_of_my_network" with your SSID  network 


# Reference
This project derivates from [Example 2 of esp32-asyncwebserver-fileupload-example](https://github.com/smford/esp32-asyncwebserver-fileupload-example), but adjusted to read filles from SD Card instead of SPIFFS and added support to navigate through folders.

# Limitations
* It uploads files only into root folder
* Restart ESP option doesn't work yet
  
