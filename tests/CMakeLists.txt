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
add_scrabble_test(test_dictionary unit/test_dictionary.c)
add_scrabble_test(test_solver unit/test_solver.c)
add_scrabble_test(test_version unit/test_version.c)
target_compile_definitions(test_version PRIVATE
  SCRABBLE_TEST_PROJECT_VERSION="${PROJECT_VERSION}"
)

if(TARGET scrabble_ui_support)
  add_executable(test_resources unit/test_resources.c)
  target_link_libraries(test_resources PRIVATE
    scrabble_core
    scrabble_ui_support
  )
  target_include_directories(test_resources PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support
  )
  add_test(NAME test_resources COMMAND test_resources)
endif()
