#!/bin/bash 
g++ OCVCapture.cpp camera.cpp -O2 -o default_camera `pkg-config --cflags --libs opencv` -lm -v4l2 -ljpeg
g++ OCVCapture.cpp camera.cpp -o neon_camera `pkg-config --cflags --libs opencv` -lm -v4l2 -ljpeg -mcpu=cortex-a8 -mtune=cortex-a8 -march=armv7-a -mfpu=neon -mfloat-abi=hard -funsafe-math-optimizations -fomit-frame-pointer -ffast-math -funroll-loops -funsafe-loop-optimizations
