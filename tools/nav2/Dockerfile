ARG ROS_DISTRO=humble
FROM ros:${ROS_DISTRO}-ros-base

# Propagate ROS_DISTRO to runtime
ARG ROS_DISTRO
ENV ROS_DISTRO=${ROS_DISTRO}

SHELL ["/bin/bash", "-lc"]
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
      ros-jazzy-navigation2 \
      ros-jazzy-nav2-bringup \
      ros-jazzy-turtlebot3* \
    && rm -rf /var/lib/apt/lists/*