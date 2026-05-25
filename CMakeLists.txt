cmake_minimum_required(VERSION 3.22.0)

project(null VERSION 1.0.0 LANGUAGES CXX)
set(EngineTarget "null_engine")
set(ThirdPartyDir "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
set(SourceDir "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(ShadersDir "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(BinDir "${CMAKE_BINARY_DIR}/bin")
set(BinShaderDir "${BinDir}/shaders")

# generate compile_commands.json for nvim's lsp
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------------------------------------------------
# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("${ThirdPartyDir}/glfw")

# ----------------------------------------------------------------------
# Vulkan
find_package(Vulkan REQUIRED)

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
# The Engine
add_executable(${EngineTarget})

file(GLOB_RECURSE ENGINE_SOURCE_FILES
  LIST_DIRECTORIES false
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
  source/*.h?? source/*.c??
)

file(GLOB_RECURSE ENGINE_SHADERS
    LIST_DIRECTORIES false
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    shaders/*.frag shaders/*.vert shaders/*.comp
)

set(ENGINE_UTILS
  "${ThirdPartyDir}/util/glm.natvis"
  # add any other util files you want to add to the IDE (for example TODO)
)

target_sources(${EngineTarget} PRIVATE ${ENGINE_SOURCE_FILES} ${ENGINE_SHADERS} ${ENGINE_UTILS})
set_source_files_properties(${ENGINE_SHADERS} ${ENGINE_UTILS} PROPERTIES HEADER_FILE_ONLY TRUE)
target_include_directories(${EngineTarget} PRIVATE ${SourceDir})
target_link_libraries(${EngineTarget} PRIVATE glm cgltf glfw Vulkan::Vulkan)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${EngineTarget} PRIVATE -Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${EngineTarget} PRIVATE /W4)
endif()

set_target_properties(${EngineTarget} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BinDir}
)
# ----------------------------------------------------------------------
# Shaders
message(STATUS "glslc: ${Vulkan_GLSLC_EXECUTABLE}")
set(ShaderCompiler ${Vulkan_GLSLC_EXECUTABLE})
file(MAKE_DIRECTORY ${BinShaderDir})

function(compile_shader shader_file)
  cmake_path(GET shader_file FILENAME shader_name)
  set(spirv_file "${BinShaderDir}/${shader_name}.spv")
  add_custom_command(
    OUTPUT ${spirv_file}
    COMMAND ${ShaderCompiler} ${shader_file} -o ${spirv_file}
    DEPENDS ${shader_file}
    COMMENT "Compiling shader ${shader_name} to SPIR-V"
  )
  set(CompiledShaders ${CompiledShaders} ${spirv_file} PARENT_SCOPE)
endfunction()

foreach(shader_file ${ENGINE_SHADERS})
  compile_shader("${CMAKE_CURRENT_SOURCE_DIR}/${shader_file}")
endforeach()

add_custom_target(
	Shaders
	DEPENDS ${CompiledShaders}
)

add_dependencies(${EngineTarget} Shaders)

message(STATUS "✅ ${EngineTarget} configured with C++${CMAKE_CXX_STANDARD}")
