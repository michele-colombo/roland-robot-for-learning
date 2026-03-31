#!/bin/bash
set -e

source /opt/ros/jazzy/setup.bash

exec ros2 launch /GIT/roland-robot-for-learning/bringup/pc/joy.launch.py
