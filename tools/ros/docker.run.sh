xhost +
docker run -it --rm --privileged \
    --network=host --ipc=host --pid=host \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    --name ros \
ros:jazzy
