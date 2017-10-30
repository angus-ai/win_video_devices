cmake_minimum_required(VERSION 2.8.12)
project(win_devices)

add_subdirectory(pybind11)
pybind11_add_module(win_devices src/devices.cpp)