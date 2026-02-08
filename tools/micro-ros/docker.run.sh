SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
source $SCRIPT_DIR/settings.sh

# Parse detached flag first so it doesn't end up inside the command
DETACHED_MODE=""
if [[ "$1" == "-d" ]]; then
    DETACHED_MODE="-d"
    shift
fi

# Recompute CMD after shifting any flags (default to bash)
CMD="${*:-bash}"

# Determine a unique container name: uros, uros_1, uros_2, ...
CONTAINER_BASENAME="uros"
CONTAINER_NAME="$CONTAINER_BASENAME"
i=0
while [ -n "$(docker ps -a -q -f name="^/${CONTAINER_NAME}$")" ]; do
    i=$((i+1))
    CONTAINER_NAME="${CONTAINER_BASENAME}_${i}"
done

# Use interactive TTY only when not detached
DOCKER_TTY_FLAGS=""
if [[ -z "$DETACHED_MODE" ]]; then
    DOCKER_TTY_FLAGS="-it"
fi

docker run $DOCKER_TTY_FLAGS $DETACHED_MODE --rm --privileged \
    --network host --ipc=host --pid=host \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v /dev:/dev \
    -v $SCRIPT_DIR:/micro-ros \
    -w /micro-ros \
    --name "$CONTAINER_NAME" \
micro-ros-agent:${DOCKER_ROS_DISTRO} bash -c "$CMD"

# Note: binding /dev is not strictly required, but allows you to use serial/by-id instead of explicit /dev/ttyXXX paths