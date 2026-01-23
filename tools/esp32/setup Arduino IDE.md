# Setup Arduino IDE (for ESP32)
## Install
Easiest way to get Arduino IDE is probably using flatpak.

```shell
flatpak install flathub cc.arduino.IDE2
```

You may need to add flathub to the current user before (only needed once):
```shell
flatpak remote-add --user --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
```

Add current user to dialout:
```shell
sudo usermod -a -G dialout $USER
```
Then it is necessary to log out and back in.

## Setup for ESP32
 You can check [this tutorial](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/) as reference.

 Basically it's adding the following line in File>Preferences>Additionl Boards Managers URLs:
 ```
 https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json, http://arduino.esp8266.com/stable/package_esp8266com_index.json
 ```

 and installing `esp32` board manager by `Expressif Systems` (from Tools > Board > Boards Manager)

 Then select the board. Currently I'm testing with an ESP32-WROOM-32 (from AZ-Delivery) and it works by selecting ESP32-WROOM-DA.

 ## Troubleshooting
 ### Apparent flash failure
 Even if you get an output like this:
```
Writing at 0x000e4078 [============================> ]  98.1% 557056/567916 bytes... 

Writing at 0x000e7ce0 [==============================] 100.0% 567916/567916 bytes... 
Wrote 883936 bytes (567916 compressed) at 0x00010000 in 9.0 seconds (788.8 kbit/s).
Hash of data verified.

Hard resetting via RTS pin...
```
the flashing was actually successful.

### Flash failure
Sometimes flashing fails the first time, usually trying again solves the issue.