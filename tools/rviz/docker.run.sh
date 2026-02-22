SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GIT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"

xhost +
docker run -it --rm --privileged \
    --network=host --ipc=host --pid=host \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v $GIT_DIR:/GIT \
    --name rviz \
rviz:jazzy
