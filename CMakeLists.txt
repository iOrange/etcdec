﻿# CMakeList.txt : CMake project for bcdec, include source and define
# project specific logic here.
# 
cmake_minimum_required(VERSION 3.8)

project("etcdec" LANGUAGES C)

# Add source to this project's executable.
add_executable(etcdec "etcdec.h" "test.c")

# TODO: Add tests and install targets if needed.
