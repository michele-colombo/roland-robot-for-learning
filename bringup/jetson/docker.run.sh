SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

docker run -it --rm --privileged \
    --net=host --ipc=host --pid=host \
    --gpus all \
    -v "$REPO_ROOT":"/GIT/roland-robot-for-learning" \
    --name roland \
ros:jazzy