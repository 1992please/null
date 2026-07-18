cmake_minimum_required(VERSION 3.22.0)
project(cgltf LANGUAGES C CXX)

add_library(cgltf STATIC cgltf.cpp cgltf/cgltf.h)
target_include_directories(cgltf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
