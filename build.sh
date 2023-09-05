#!/bin/sh
clang++ main.cpp -std=c++11 `pkg-config --libs --cflags raylib` -o Game