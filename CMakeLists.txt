cmake_minimum_required(VERSION 3.22.0)
project(null VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # generate compile_commands.json for nvim's lsp
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EngineTarget "null_engine")
set(ThirdPartyDir "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
set(SourceDir "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(ShadersDir "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(BinDir "${CMAKE_BINARY_DIR}/bin")
set(RelativeBinShaderDirctory "compiled_shaders")
set(BinShaderDir "${BinDir}/${RelativeBinShaderDirctory}")

# ----------------------------------------------------------------------
# Vulkan
find_package(Vulkan REQUIRED COMPONENTS volk)
find_program(Vulkan_SLANGC_EXECUTABLE slangc HINTS $ENV{VULKAN_SDK}/bin REQUIRED)
message(STATUS "slang: ${Vulkan_SLANGC_EXECUTABLE}")

# ----------------------------------------------------------------------
# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("${ThirdPartyDir}/glfw")

# ----------------------------------------------------------------------
# GLM
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE "${ThirdPartyDir}/glm")
target_compile_definitions(glm INTERFACE GLM_FORCE_DEPTH_ZERO_TO_ONE)

# ----------------------------------------------------------------------
# cgltf
add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE "${ThirdPartyDir}/cgltf")

# ----------------------------------------------------------------------
# spdlog
add_library(spdlog INTERFACE)
target_include_directories(spdlog INTERFACE "${ThirdPartyDir}/spdlog")

# ----------------------------------------------------------------------
# The Engine
add_executable(${EngineTarget})

file(GLOB_RECURSE ENGINE_SOURCE_FILES
  LIST_DIRECTORIES false
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
  src/*.h src/*.cpp
)

file(GLOB_RECURSE ENGINE_SHADERS
    LIST_DIRECTORIES false
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    shaders/*.slang  shaders/*.comp
)

if(MSVC)
    set(ENGINE_UTILS "${ThirdPartyDir}/glm/util/glm.natvis")
else()
    set(ENGINE_UTILS "")
endif()

target_sources(${EngineTarget} PRIVATE ${ENGINE_SOURCE_FILES} ${ENGINE_SHADERS} ${ENGINE_UTILS})
set_source_files_properties(${ENGINE_SHADERS} ${ENGINE_UTILS} PROPERTIES HEADER_FILE_ONLY TRUE)
target_include_directories(${EngineTarget} PRIVATE ${SourceDir})
target_link_libraries(${EngineTarget} PRIVATE
  Vulkan::volk
  glfw
  glm
  cgltf
  spdlog
)
target_compile_definitions(${EngineTarget} PRIVATE
  $<$<CONFIG:Debug>:NE_BUILD_DEBUG>
  $<$<CONFIG:RelWithDebInfo>:NE_BUILD_DEVELOPMENT>
  $<$<CONFIG:Release>:NE_BUILD_SHIPPING>

  $<$<PLATFORM_ID:Windows>:NE_PLATFORM_WINDOWS>
  $<$<PLATFORM_ID:Linux>:NE_PLATFORM_LINUX>
  $<$<PLATFORM_ID:Darwin>:NE_PLATFORM_APPLE>

  NE_BUILD_DIR="${CMAKE_BINARY_DIR}"
  NE_SHADER_DIR="${RelativeBinShaderDirctory}"

  VK_NO_PROTOTYPES
)

target_compile_options(${EngineTarget} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8 /Zc:preprocessor /MP>
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall;-Wextra;-Wpedantic>
)

set_target_properties(${EngineTarget} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BinDir})

# ----------------------------------------------------------------------
# Shaders

file(MAKE_DIRECTORY ${BinShaderDir})
set(CompiledShaders "")

foreach(shader_file ${ENGINE_SHADERS})
  get_filename_component(shader_name ${shader_file} NAME_WE)
  set(spirv_file "${BinShaderDir}/${shader_name}.spv")
  set(absolute_shader_path "${CMAKE_CURRENT_SOURCE_DIR}/${shader_file}")
  message(STATUS "SLANGC compile: ${absolute_shader_path}")
  add_custom_command(
    OUTPUT ${spirv_file}
    COMMAND ${Vulkan_SLANGC_EXECUTABLE} ${absolute_shader_path}
            -target spirv
            -profile spirv_1_4
            -emit-spirv-directly
            -fvk-use-entrypoint-name
            -o ${spirv_file}
    DEPENDS ${absolute_shader_path}
    COMMENT "compiling shader ${shader_name} to SPIR-V"
  )
  list(APPEND CompiledShaders ${spirv_file})
endforeach()

add_custom_target(
	Shaders
	DEPENDS ${CompiledShaders}
)

add_dependencies(${EngineTarget} Shaders)

# ----------------------------------------------------------------------
# Visual Studio
if(MSVC)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EngineTarget})
  set_property(TARGET ${EngineTarget} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${BinDir})
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_SOURCE_FILES} ${ENGINE_SHADERS})
  source_group("Misc" FILES ${ENGINE_UTILS})
  set_target_properties(glfw PROPERTIES FOLDER "Misc/Thirdparty/GLFW3")
  set_target_properties(update_mappings PROPERTIES FOLDER "Misc/Thirdparty/GLFW3")
  set_target_properties(uninstall PROPERTIES FOLDER "Misc/Thirdparty/GLFW3")
  set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Misc/CMakeTargets")
  set_property(TARGET ${EngineTarget} PROPERTY FOLDER Engine)
  set_property(TARGET Shaders PROPERTY FOLDER Engine)
endif()

message(STATUS "✅ ${EngineTarget} configured with C++${CMAKE_CXX_STANDARD}")
