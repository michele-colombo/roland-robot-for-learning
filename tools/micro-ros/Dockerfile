# Build micro-ROS agent environment using /micro-ros_ws
# You can override ROS_DISTRO at build time (e.g., --build-arg ROS_DISTRO=iron)
ARG ROS_DISTRO=humble
FROM ros:${ROS_DISTRO}-ros-base

# Propagate ROS_DISTRO to runtime
ARG ROS_DISTRO
ENV ROS_DISTRO=${ROS_DISTRO}

SHELL ["/bin/bash", "-lc"]
ENV DEBIAN_FRONTEND=noninteractive

# Base tools and ROS dev utilities
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      git \
      python3-pip \
      python3-rosdep \
      python3-colcon-common-extensions \
    && rm -rf /var/lib/apt/lists/*

# Initialize rosdep (ok if already initialized) and update
RUN apt-get update && (rosdep init || true) && rosdep update \
    && rm -rf /var/lib/apt/lists/*

# Workspace at /micro-ros_ws (requested)
WORKDIR /micro-ros_ws

# Fetch micro_ros_setup for the selected ROS_DISTRO
RUN mkdir -p src && \
    git clone -b ${ROS_DISTRO} https://github.com/micro-ROS/micro_ros_setup.git src/micro_ros_setup

# Resolve dependencies for sources (ignore-src for system deps only)
RUN apt-get update && rosdep update && \
    rosdep install --from-paths src --ignore-src -r -y \
    && rm -rf /var/lib/apt/lists/*

# Build overlay containing micro_ros_setup
RUN source /opt/ros/${ROS_DISTRO}/setup.bash \
    && colcon build

# Create and build the micro-ROS Agent workspace
RUN source /opt/ros/${ROS_DISTRO}/setup.bash \
    && source /micro-ros_ws/install/local_setup.bash \
    && ros2 run micro_ros_setup create_agent_ws.sh
RUN source /opt/ros/${ROS_DISTRO}/setup.bash \
    && source /micro-ros_ws/install/local_setup.bash \
    && ros2 run micro_ros_setup build_agent.sh

# Copy and set entrypoint
COPY entrypoint.sh /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]
