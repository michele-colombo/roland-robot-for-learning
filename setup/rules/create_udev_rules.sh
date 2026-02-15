#!/bin/bash

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")

echo "start copy robot.rules to  /etc/udev/rules.d/"
sudo cp "$SCRIPT_DIR/robot.rules" /etc/udev/rules.d/
echo " "
echo "Restarting udev"
echo ""
sudo udevadm control --reload-rules
sudo service udev restart
sudo udevadm trigger
echo "finish "
