#!/usr/bin/env bash
# hatch.sh energy: borrows an Amazon Linux egg, hatches a pigeon, crams it into a zip.
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p build
docker run --rm --platform linux/arm64 -v "$PWD":/coop -w /coop amazonlinux:2023 bash -c '
  dnf install -y -q --allowerasing gcc-c++ libcurl-devel >/dev/null &&
  g++ -std=c++17 -O2 -Wall -Wextra -o build/bootstrap relay.cpp -lcurl'
chmod +x build/bootstrap
rm -f function.zip && (cd build && zip -q ../function.zip bootstrap)
echo "pigeon packed: function.zip"
