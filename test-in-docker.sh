#/bin/env bash

if [ ! -d .git ]; then
    echo "This script must be run in the project's root directory (as ./test-in-docker.sh)" >/dev/stderr
    exit 1
fi

set -e
docker build ./tests/docker-gtest -t d-interp-tester
docker run -it -v ./src:/project -v ./docker-testbuild:/build --rm d-interp-tester
