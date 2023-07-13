#!/bin/bash

set -eu

if [[ ! -f Build/Makefile ]]; then
	rm -fr Build
fi

if [[ ! -d Build ]]; then
	cmake -B Build
fi

ok=0
cmake --build Build && ok=1

if [[ $ok -eq 1 ]]; then
	Build/parrssl --analyze-rules 2>&1 | less -R +g
else
	cmake --build Build 2>&1 | less -R +g
fi
