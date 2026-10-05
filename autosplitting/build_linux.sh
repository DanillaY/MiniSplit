#!/bin/sh
g++ main.cpp -o autosplit \
    -O3 -march=native -mtune=native \
    -flto -pthread \
    -lboost_filesystem -lboost_thread
