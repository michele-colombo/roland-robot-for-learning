#!/bin/bash

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")

echo "sudo rm   /etc/udev/rules.d/robot.rules"
sudo rm /etc/udev/rules.d/robot.rules
echo " "
echo "Restarting udev"
echo ""
sudo udevadm control --reload-rules
sudo service udev restart
sudo udevadm trigger
echo "finish  delete"
