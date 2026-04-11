ARG ROS_DISTRO=jazzy
FROM ros:${ROS_DISTRO}-ros-base

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        python3-colcon-common-extensions \
        ros-${ROS_DISTRO}-joy \
        ros-${ROS_DISTRO}-teleop-twist-joy \
    && rm -rf /var/lib/apt/lists/*
