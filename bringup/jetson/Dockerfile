ARG ROS_DISTRO=jazzy
FROM ros:${ROS_DISTRO}-ros-base

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        python3-colcon-common-extensions \
    && rm -rf /var/lib/apt/lists/*
