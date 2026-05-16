cmake_minimum_required(VERSION 3.22.0)

project(null VERSION 1.0.0 LANGUAGES CXX)
set(EngineTarget "null_engine")
set(ThirdPartyFolder "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

# generate compile_commands.json for nvim's lsp
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QT6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Vulkan
find_package(Vulkan REQUIRED)

# GLM
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${ThirdPartyFolder}/glm)
target_compile_definitions(glm INTERFACE GLM_FORCE_DEPTH_ZERO_TO_ONE)

# cgltf
add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE ${ThirdPartyFolder}/cgltf)

# The Engine
add_executable(${EngineTarget} src/main.cpp)
target_link_libraries(${EngineTarget} PRIVATE
  glm
  cgltf
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets
  Vulkan::Vulkan
)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${EngineTarget} PRIVATE -Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${EngineTarget} PRIVATE /W4)
endif()

# Set output directory
set_target_properties(${EngineTarget} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

message(STATUS "✅ ${EngineTarget} configured with C++${CMAKE_CXX_STANDARD}")
