SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

source $SCRIPT_DIR/settings.sh

docker build \
  -t micro-ros-agent:${DOCKER_ROS_DISTRO} \
  --build-arg ROS_DISTRO=${DOCKER_ROS_DISTRO} \
  --file $SCRIPT_DIR/Dockerfile \
  $SCRIPT_DIR