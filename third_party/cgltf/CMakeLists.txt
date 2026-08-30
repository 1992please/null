cmake_minimum_required(VERSION 3.22.0)

project(cgltf LANGUAGES C CXX)
add_library(cgltf INTERFACE)
target_sources(cgltf INTERFACE
  cgltf.cpp
  cgltf.h
)

target_include_directories(cgltf INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
)
