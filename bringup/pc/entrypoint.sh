#!/bin/bash
set -e

source /opt/ros/${ROS_DISTRO}/setup.bash

if [ -f /roland/ros2_wss/pc/install/setup.bash ]; then
    source /roland/ros2_wss/pc/install/setup.bash
fi

exec "$@"
