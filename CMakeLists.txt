cmake_minimum_required(VERSION 3.20)
project(scrabble_solver VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(SCRABBLE_BUILD_GUI "Build the GTK 4 desktop application" ON)

add_library(scrabble_core
  src/core/dictionary.c
  src/core/rack.c
  src/core/scoring.c
  src/core/solver.c
  src/core/version.c
)
target_include_directories(scrabble_core PUBLIC include)

if(SCRABBLE_BUILD_GUI)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

  add_executable(scrabble_solver WIN32
    src/main.c
    src/ui/application.c
    src/ui/main_window.c
    src/ui/result_list.c
  )
  target_include_directories(scrabble_solver PRIVATE src)
  target_link_libraries(scrabble_solver PRIVATE
    scrabble_core
    PkgConfig::GTK4
  )
endif()

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
