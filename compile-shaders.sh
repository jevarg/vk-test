#!/usr/bin/env bash

set -e

cd shaders
/c/VulkanSDK/1.3.290.0/Bin/glslc tri.vert -o vert.spv
/c/VulkanSDK/1.3.290.0/Bin/glslc tri.frag -o frag.spv
cd ..

echo "OK"