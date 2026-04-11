#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

COMPOSE="docker compose -f $SCRIPT_DIR/docker-compose.yml"
SERVICE="roland-pc"

usage() {
    cat <<EOF
Usage: $(basename "$0") <command>

Commands:
  build   Build the ROS2 workspace (creates pkg symlinks + colcon build)
  start   Start all services in background
  stop    Stop all services
  shell   Open a new interactive shell with workspace sourced
  attach  Attach to the running container with workspace sourced
  logs    Follow service logs
EOF
    exit 1
}

case "${1:-}" in
    build)
        $COMPOSE run --rm --no-deps "$SERVICE" bash -c '
            set -e
            mkdir -p /roland/ros2_wss/pc/src
            ln -sfn /roland/software/pkgs/gimbal_control /roland/ros2_wss/pc/src/
            cd /roland/ros2_wss/pc
            colcon build --symlink-install
        '
        ;;
    start)
        $COMPOSE up -d
        echo "Services started. Use '$(basename "$0") logs' to follow output, '$(basename "$0") stop' to stop."
        ;;
    stop)
        $COMPOSE down
        ;;
    shell)
        $COMPOSE run --rm --no-deps "$SERVICE" bash
        ;;
    attach)
        $COMPOSE exec "$SERVICE" bash -c '
            source /opt/ros/${ROS_DISTRO}/setup.bash
            [ -f /roland/ros2_wss/pc/install/setup.bash ] && source /roland/ros2_wss/pc/install/setup.bash
            exec bash
        '
        ;;
    logs)
        $COMPOSE logs -f
        ;;
    *)
        usage
        ;;
esac
