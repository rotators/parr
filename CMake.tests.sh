#!/bin/bash

set -e

if [[ ! -f Build/Makefile ]]; then
	rm -fr Build
fi

if [[ ! -d Build ]]; then
	cmake -B Build
fi

cmake --build Build --target parrssl-test
