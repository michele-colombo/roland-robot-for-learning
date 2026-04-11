# Setup

## Jetson

- Connect base ESP32, gimbal ESP32, ZED and LiDAR to USB ports of Jetson
- Since both my ESP32 and LiDAR use the same CP2102 chip I cannot use `serial/by-id` to distinguish between them. Therefore I rely on their stable physical connection (you must connect them always to the same port) and use the USB path to the port they are connected to.
    - In order to find the correct USB port path you can use `udevadm monitor --property | grep DEVPATH` and plug/unplug the different devices.
    - Set their address in `setup/jetson/rules/robot.rules`.
    - Run `./setup/jetson/rules/created_udev_rules.sh` to install the rules
    - You should now see `/dev/esp32-base`, `/dev/esp32-gimbal` and `/dev/ldlidar` (with all the devices connected).