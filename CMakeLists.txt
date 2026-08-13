cmake_minimum_required(VERSION 3.20)
project(scrabble_solver VERSION 0.1.0 LANGUAGES C)

include(GNUInstallDirs)

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

  add_library(scrabble_ui_support STATIC
    src/platform/resources.c
  )
  target_include_directories(scrabble_ui_support PUBLIC src)
  target_compile_definitions(scrabble_ui_support PRIVATE
    SCRABBLE_SOURCE_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
    SCRABBLE_INSTALL_DATA_DIR="${CMAKE_INSTALL_FULL_DATADIR}/scrabble-solver"
  )
  target_link_libraries(scrabble_ui_support PUBLIC PkgConfig::GTK4)

  add_executable(scrabble_solver WIN32
    src/main.c
    src/ui/application.c
    src/ui/main_window.c
    src/ui/rack_view.c
    src/ui/result_list.c
    src/ui/theme.c
  )
  target_include_directories(scrabble_solver PRIVATE src)
  target_link_libraries(scrabble_solver PRIVATE
    scrabble_core
    scrabble_ui_support
  )

  install(TARGETS scrabble_solver RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  install(
    DIRECTORY assets/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/scrabble-solver
  )
endif()

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
