#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

NAME_BASENAME="${1:-uros}"
PATTERN="^/${NAME_BASENAME}(_.*)?$"

# Find running containers matching the basename
running=$(docker ps -q -f name="$PATTERN")
if [[ -z "$running" ]]; then
    echo "No running containers matching basename '$NAME_BASENAME'."
else
    echo "Stopping containers: $running"
    docker stop $running
fi

# Remove any leftover containers (stopped or exited) matching the basename
all=$(docker ps -a -q -f name="$PATTERN")
if [[ -n "$all" ]]; then
    echo "Removing containers: $all"
    docker rm $all || true
fi

echo "Done."
