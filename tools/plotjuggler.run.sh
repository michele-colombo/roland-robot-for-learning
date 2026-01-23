xhost +
docker run -it --rm --privileged \
    --network=host --ipc=host --pid=host \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
ros-pj:jazzy \
    bash -c \
        "source /opt/ros/jazzy/setup.bash && \
        ros2 run plotjuggler plotjuggler"