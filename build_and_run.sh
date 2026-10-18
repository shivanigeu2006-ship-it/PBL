#!/bin/sh
# Linux / macOS. Run from the SmartStock folder:  ./build_and_run.sh
set -e
g++ -std=c++17 -O2 -Iinclude -Isrc src/main.cpp -pthread -o smartstock
echo "Open http://localhost:8080 in your browser"
./smartstock
