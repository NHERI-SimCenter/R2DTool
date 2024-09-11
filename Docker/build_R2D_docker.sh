#!/bin/bash

echo "Starting R2D Docker build"

docker build --platform linux/amd64 -t r2d -f ./Dockerfile .
