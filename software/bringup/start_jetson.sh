SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

$REPO_ROOT/tools/micro-ros/docker.run.sh .run_agent.sh \
    serial --dev /dev/ttyUSB0

$REPO_ROOT/tools/micro-ros/docker.run.sh .run_agent.sh \
    serial --dev /dev/ttyUSB1

