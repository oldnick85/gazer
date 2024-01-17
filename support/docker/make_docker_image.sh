#!/bin/bash
TAG_VERSION=$(git describe --tags || echo "unknown")
docker build --file="support/Dockerfile" \
    --build-arg UBUNTU_VERSION=23.10 \
    --build-arg VERSION=2.50.2 \
    --build-arg GAZER_COMPILER=gcc \
    --tag gazer:${TAG_VERSION} \
    --tag gazer:latest \
    .
