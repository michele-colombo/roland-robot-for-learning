#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "start copy robot.rules to  /etc/udev/rules.d/"
sudo cp "$SCRIPT_DIR/robot.rules" /etc/udev/rules.d/
echo " "
echo "Restarting udev"
echo ""
sudo udevadm control --reload-rules
sudo service udev restart
sudo udevadm trigger
echo "finish "