cmake_minimum_required(VERSION 3.20)
project(scrabble_solver VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(scrabble_core
  src/core/scoring.c
  src/core/version.c
)
target_include_directories(scrabble_core PUBLIC include)

add_executable(scrabble_solver src/main.c)
target_link_libraries(scrabble_solver PRIVATE scrabble_core)
