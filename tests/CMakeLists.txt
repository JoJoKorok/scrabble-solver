function(add_scrabble_test test_name source_file)
  add_executable(${test_name} ${source_file})
  target_link_libraries(${test_name} PRIVATE scrabble_core)
  target_include_directories(${test_name} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    ${CMAKE_CURRENT_BINARY_DIR}/generated
  )
  add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/fixtures)
configure_file(
  fixtures/dictionary.txt
  ${CMAKE_CURRENT_BINARY_DIR}/fixtures/dictionary.txt
  COPYONLY
)
configure_file(
  fixtures/solver_dictionary.txt
  ${CMAKE_CURRENT_BINARY_DIR}/fixtures/solver_dictionary.txt
  COPYONLY
)
configure_file(
  support/test_paths.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/generated/test_paths.h
  @ONLY
)

add_scrabble_test(test_scoring unit/test_scoring.c)
add_scrabble_test(test_rack unit/test_rack.c)
add_scrabble_test(test_board unit/test_board.c)
add_scrabble_test(test_game unit/test_game.c)
add_scrabble_test(test_move unit/test_move.c)
add_scrabble_test(test_placement unit/test_placement.c)
add_scrabble_test(test_dictionary unit/test_dictionary.c)
target_include_directories(test_dictionary PRIVATE
  ${PROJECT_SOURCE_DIR}/src/core
)
add_scrabble_test(test_solver unit/test_solver.c)
add_scrabble_test(test_version unit/test_version.c)
target_compile_definitions(test_version PRIVATE
  SCRABBLE_TEST_PROJECT_VERSION="${PROJECT_VERSION}"
)

add_scrabble_test(
  test_large_dictionary
  performance/test_large_dictionary.c
)
target_include_directories(test_large_dictionary PRIVATE
  ${PROJECT_SOURCE_DIR}/src/core
)
set_tests_properties(test_large_dictionary PROPERTIES
  LABELS "dictionary;performance"
  TIMEOUT 15
)

if(TARGET scrabble_ui_support)
  add_executable(test_suggestion_placement ui/test_suggestion_placement.c)
  target_link_libraries(test_suggestion_placement PRIVATE scrabble_ui)
  target_include_directories(test_suggestion_placement PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated
  )
  find_program(SCRABBLE_DBUS_RUN_SESSION NAMES dbus-run-session)
  if(SCRABBLE_DBUS_RUN_SESSION)
    add_test(NAME test_suggestion_placement
      COMMAND ${SCRABBLE_DBUS_RUN_SESSION} -- ./$<TARGET_FILE_NAME:test_suggestion_placement>)
  else()
    add_test(NAME test_suggestion_placement COMMAND test_suggestion_placement)
  endif()
  set_tests_properties(test_suggestion_placement PROPERTIES
    LABELS "ui;integration"
    WORKING_DIRECTORY $<TARGET_FILE_DIR:test_suggestion_placement>
    SKIP_RETURN_CODE 77
    TIMEOUT 30
    ENVIRONMENT "GTK_A11Y=none;GSK_RENDERER=cairo"
  )

  add_executable(test_resources unit/test_resources.c)
  target_link_libraries(test_resources PRIVATE
    scrabble_core
    scrabble_ui_support
  )
  target_include_directories(test_resources PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support
  )
  add_test(NAME test_resources COMMAND test_resources)

  add_executable(test_settings unit/test_settings.c)
  target_link_libraries(test_settings PRIVATE scrabble_ui_support)
  target_include_directories(test_settings PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support
  )
  add_test(NAME test_settings COMMAND test_settings)
endif()
