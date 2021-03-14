cmake_minimum_required(VERSION 3.6)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(RMSCALC LANGUAGES C DESCRIPTION "RMS Calculator Library")

add_compile_options(
  -O0
  -g3
  -ggdb
  -ffunction-sections
  -fdata-sections
  -fPIC
  -Wall
  -Wfatal-errors
  -funsigned-char
)

set(SOURCES "src/rmscalc.c")
add_library(rmscalc SHARED ${SOURCES})
