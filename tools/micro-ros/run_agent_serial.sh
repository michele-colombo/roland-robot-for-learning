#!/usr/bin/env bash
set -e

port=$(readlink -f /dev/serial/by-id/usb-Silicon_Labs* 2>/dev/null)
echo "Found ESP32 (CP210x chip) on port: $port"

ros2 run micro_ros_agent micro_ros_agent serial --dev $port