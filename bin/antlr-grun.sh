#!/bin/bash

set -eu

java -cp "Build/ANTLR/*.jar" org.antlr.v4.gui.TestRig $*
