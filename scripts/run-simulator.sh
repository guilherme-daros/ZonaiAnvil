#!/bin/bash

# Script to build and run the ZonaiSimulator

# 1. Build project
./scripts/build.sh
if [ $? -ne 0 ]; then
    echo "[ERROR] Build failed."
    exit 1
fi

# 2. Run Simulator
# Note: The simulator will call scripts/setup-serialsim.sh internally if port is missing
./build/ZonaiSimulator /dev/ttySIM0 115200
