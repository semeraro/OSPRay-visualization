## top level cmake lists file for the OSPRayVisualization project. 
## the project builds OSPRay based visualizations  
## from here in the src directory. The build directory is ../build

cmake_minimum_required(VERSION 3.1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

project(OSPRayVisualization LANGUAGES CXX)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON) # no in sourc builds allowed
# targets created in the subdirectories go to the ../build
# (PROJECT_BINARY_DIR) directory. This keeps all the libs etc in
# one place.

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})