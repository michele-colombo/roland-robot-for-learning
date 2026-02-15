SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

DOCKER_ROS_DISTRO=${1:-"jazzy"}

docker build \
  -t nav2:${DOCKER_ROS_DISTRO} \
  --build-arg ROS_DISTRO=${DOCKER_ROS_DISTRO} \
  --file $SCRIPT_DIR/Dockerfile \
  $SCRIPT_DIR