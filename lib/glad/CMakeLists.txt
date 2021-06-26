cmake_minimum_required(VERSION 3.15)
project(glad VERSION 4.6 LANGUAGES C)

# https://glad.dav1d.de/
# Language: C/C++
# Specification: OpenGL
# Api:
# - gl: 4.5
# Profile: Core
# Extensions: <all>

add_library(glad STATIC ${PROJECT_SOURCE_DIR}/src/glad.c)

target_include_directories(glad PUBLIC ${PROJECT_SOURCE_DIR}/include/)