#!/usr/bin/env bash
set -e

BUILD_FROM="ghcr.io/home-assistant/amd64-base-python:3.11-alpine3.18"

echo "Building Solar Tracker Controller Add-on..."
echo "Architecture: amd64"
echo "Base: ${BUILD_FROM}"

docker build \
    --build-arg BUILD_FROM="${BUILD_FROM}" \
    --tag local/solar-tracker-addon:latest \
    --file Dockerfile \
    .

echo "Build complete!"
echo "Run with: docker run -p 8099:8099 local/solar-tracker-addon:latest"
