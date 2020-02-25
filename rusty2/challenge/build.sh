#!/bin/bash
set -euo pipefail

# Build
pushd src
cargo build --release
popd

# Grab the binary
cp src/target/release/rusty .
strip rusty
chmod +x rusty

# Delete the compiled stuff
rm -rf src/target

# Copy the binary to distfiles
cp rusty ../distfiles/
