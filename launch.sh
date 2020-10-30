#!/bin/bash

docker build -t opencv-45:1 .
docker run --rm -it -u $(id -u) -v "$PWD":/tmp opencv-45:1 createMagic
