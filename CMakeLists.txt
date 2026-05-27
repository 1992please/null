cmake_minimum_required(VERSION 3.22.0)
project(null VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # generate compile_commands.json for nvim's lsp
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "" FORCE)

set(EngineTarget "null_engine")
set(ThirdPartyDir "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
set(SourceDir "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(ShadersDir "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(BinDir "${CMAKE_BINARY_DIR}/bin")
set(BinShaderDir "${BinDir}/shaders")


# ----------------------------------------------------------------------
# Vulkan
find_package(Vulkan REQUIRED)

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
  src/*.h?? src/*.c??
)

file(GLOB_RECURSE ENGINE_SHADERS
    LIST_DIRECTORIES false
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    shaders/*.frag shaders/*.vert shaders/*.comp
)

message(STATUS "ENGINE_SOURCE_FILES: ${ENGINE_SOURCE_FILES}")
message(STATUS "ENGINE_SHADERS: ${ENGINE_SHADERS}")

set(ENGINE_UTILS
  "${ThirdPartyDir}/glm/util/glm.natvis"
  # add any other util files you want to add to the IDE (for example TODO)
)

target_sources(${EngineTarget} PRIVATE ${ENGINE_SOURCE_FILES} ${ENGINE_SHADERS} ${ENGINE_UTILS})
set_source_files_properties(${ENGINE_SHADERS} ${ENGINE_UTILS} PROPERTIES HEADER_FILE_ONLY TRUE)
target_include_directories(${EngineTarget} PRIVATE ${SourceDir})
target_link_libraries(${EngineTarget} PRIVATE
  glfw
  glm
  cgltf
  spdlog
  Vulkan::Vulkan
)
target_compile_definitions(${EngineTarget} PRIVATE
  $<$<CONFIG:Debug>:NE_DEBUG>
  $<$<CONFIG:RelWithDebInfo>:NE_RELEASE>
  $<$<CONFIG:Release>:NE_SHIPPING>
)

target_compile_options(${EngineTarget} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall;-Wextra;-Wpedantic>
)

set_target_properties(${EngineTarget} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BinDir})
# ----------------------------------------------------------------------
# Shaders
message(STATUS "glslc: ${Vulkan_GLSLC_EXECUTABLE}")
set(ShaderCompiler ${Vulkan_GLSLC_EXECUTABLE})
file(MAKE_DIRECTORY ${BinShaderDir})

set(CompiledShaders "")
foreach(shader_file ${ENGINE_SHADERS})
  cmake_path(GET shader_file FILENAME shader_name)
  set(spirv_file "${BinShaderDir}/${shader_name}.spv")
  set(absolute_shader_path "${CMAKE_CURRENT_SOURCE_DIR}/${shader_file}")
  add_custom_command(
    OUTPUT ${spirv_file}
    COMMAND ${ShaderCompiler} ${absolute_shader_path} -o ${spirv_file}
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

message(STATUS "✅ ${EngineTarget} configured with C++${CMAKE_CXX_STANDARD}")
