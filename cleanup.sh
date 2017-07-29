#!/bin/sh
make clean || true
find -type d -name "CMakeFiles" -exec rm -rf {} +
find -type f -name "cmake_install.cmake" -exec rm -rf {} +
find -type f -name "Makefile" -exec rm -rf {} +
find -type f -name "cmake.out" -exec rm -rf {} +
find -type f -name "CMakeCache.txt" -exec rm -rf {} +
find -type f -name "*.gambas" -exec rm -rf {} +
\rm -r build/* || true
