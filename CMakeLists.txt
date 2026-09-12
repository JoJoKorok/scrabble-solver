cmake_minimum_required(VERSION 3.20)
project(
  scrabble_solver
  VERSION 0.1.0
  DESCRIPTION "Cross-platform Scrabble rack solver"
  HOMEPAGE_URL "https://github.com/JoJoKorok/scrabble-solver"
  LANGUAGES C
)

include(GNUInstallDirs)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(SCRABBLE_BUILD_GUI "Build the GTK 4 desktop application" ON)
option(
  SCRABBLE_BUNDLE_WINDOWS_RUNTIME
  "Bundle the MSYS2 GTK runtime in a portable Windows package"
  OFF
)

if(SCRABBLE_BUNDLE_WINDOWS_RUNTIME AND NOT WIN32)
  message(FATAL_ERROR
    "SCRABBLE_BUNDLE_WINDOWS_RUNTIME is supported only on Windows"
  )
endif()

if(SCRABBLE_BUNDLE_WINDOWS_RUNTIME AND NOT SCRABBLE_BUILD_GUI)
  message(FATAL_ERROR
    "SCRABBLE_BUNDLE_WINDOWS_RUNTIME requires SCRABBLE_BUILD_GUI=ON"
  )
endif()

configure_file(
  cmake/version_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/generated/scrabble/version_config.h
  @ONLY
)

add_library(scrabble_core
  src/core/board.c
  src/core/dictionary.c
  src/core/game.c
  src/core/move.c
  src/core/move_analysis.c
  src/core/placement.c
  src/core/rack.c
  src/core/scoring.c
  src/core/solver.c
  src/core/version.c
)
target_include_directories(scrabble_core
  PUBLIC include
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated
)

if(SCRABBLE_BUILD_GUI)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

  add_library(scrabble_ui_support STATIC
    src/platform/resources.c
    src/platform/settings.c
  )
  target_include_directories(scrabble_ui_support PUBLIC src)
  target_compile_definitions(scrabble_ui_support PRIVATE
    SCRABBLE_SOURCE_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
    SCRABBLE_INSTALL_DATA_DIR="${CMAKE_INSTALL_FULL_DATADIR}/scrabble-solver"
  )
  target_link_libraries(scrabble_ui_support PUBLIC PkgConfig::GTK4)

  add_library(scrabble_ui STATIC
    src/ui/board_view.c
    src/ui/dictionary_picker.c
    src/ui/main_window.c
    src/ui/move_controls.c
    src/ui/rack_view.c
    src/ui/result_list.c
    src/ui/theme.c
  )
  target_include_directories(scrabble_ui PUBLIC src)
  target_link_libraries(scrabble_ui PUBLIC
    scrabble_core
    scrabble_ui_support
  )

  add_executable(scrabble_solver WIN32 src/main.c src/ui/application.c)
  target_link_libraries(scrabble_solver PRIVATE scrabble_ui)

  install(TARGETS scrabble_solver RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  install(
    DIRECTORY assets/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/scrabble-solver
  )

  if(SCRABBLE_BUNDLE_WINDOWS_RUNTIME)
    if(NOT GTK4_PREFIX)
      message(FATAL_ERROR
        "pkg-config did not report the GTK runtime prefix"
      )
    endif()
    if(NOT CMAKE_OBJDUMP)
      find_program(CMAKE_OBJDUMP NAMES objdump REQUIRED)
    endif()

    get_filename_component(
      SCRABBLE_GTK_RUNTIME_ROOT
      "${GTK4_PREFIX}"
      REALPATH
    )
    configure_file(
      cmake/install_windows_runtime.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/install_windows_runtime.cmake
      @ONLY
    )
    install(
      SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/install_windows_runtime.cmake
    )
    install(FILES LICENSE DESTINATION .)

    set(CPACK_GENERATOR "ZIP")
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
    set(CPACK_PACKAGE_FILE_NAME
      "scrabble-solver-${PROJECT_VERSION}-windows-x86_64"
    )
    set(CPACK_PACKAGE_NAME "Scrabble Solver")
    set(CPACK_PACKAGE_VENDOR "Joseph (JoJoKorok)")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
  endif()
endif()

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

if(SCRABBLE_BUNDLE_WINDOWS_RUNTIME)
  include(CPack)
endif()
