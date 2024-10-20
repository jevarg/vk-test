#!/usr/bin/env bash

set -e

PROJECT_DIR=$(dirname "$(readlink -f "$0")")
export CONAN_HOME="$PROJECT_DIR/.conan"

conan profile detect --exist-ok
conan install --build=missing --profile msvc --profile debug conanfile.txt
conan install --build=missing --profile msvc --profile release conanfile.txt
