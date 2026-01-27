cmake_minimum_required(VERSION 3.10)
project(PickleballModernRenderer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLM REQUIRED)
find_package(assimp REQUIRED)

# Include directories
include_directories(
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${ASSIMP_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/modern_renderer
)

# Source files
set(SOURCES
    modern_renderer/ModernRenderer.cpp
    modern_renderer/example_modern_renderer.cpp
)

# Executable
add_executable(modern_pickleball ${SOURCES})

# Link libraries
target_link_libraries(modern_pickleball
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    glfw
    ${ASSIMP_LIBRARIES}
)

# Copy shaders to build directory
file(COPY ${CMAKE_SOURCE_DIR}/modern_renderer/shaders 
     DESTINATION ${CMAKE_BINARY_DIR}/modern_renderer)

# Copy resources (if any)
# file(COPY ${CMAKE_SOURCE_DIR}/textures 
#      DESTINATION ${CMAKE_BINARY_DIR})

message(STATUS "Modern Pickleball Renderer configured successfully!")
message(STATUS "  OpenGL: ${OPENGL_LIBRARIES}")
message(STATUS "  GLEW: ${GLEW_LIBRARIES}")
message(STATUS "  Assimp: ${ASSIMP_LIBRARIES}")
