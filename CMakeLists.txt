cmake_minimum_required(VERSION 3.10)
project(fluidSim VERSION 1.0.0)
cmake_policy(SET CMP0071 NEW)

# Add custom cmake module path for FindGLFW3.cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Add the source directory
add_subdirectory(src)