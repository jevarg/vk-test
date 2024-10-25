#!/usr/bin/env bash

set -e

cd shaders
glslc tri.vert -o vert.spv
glslc tri.frag -o frag.spv
cd ..

echo "OK"