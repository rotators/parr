#!/bin/bash

set -e

./CMake.sh || exit 1

cmake --build Build --target parrssl-test
