#!/usr/bin/env bash

set -e

OS_NAME=$(uname -s)

case "$OS_NAME" in
  Darwin*)
    export PROFILE="apple-silicon"
    ;;
  *)
    echo "Unsupported platform! $OS_NAME"
    exit 1
esac

PROJECT_DIR=$(dirname "$(readlink -f "$0")")
export CONAN_HOME="$PROJECT_DIR/.conan"

conan profile detect --exist-ok
conan install --build=missing --profile "$PROFILE" --profile debug conanfile.txt
conan install --build=missing --profile "$PROFILE" --profile release conanfile.txt
