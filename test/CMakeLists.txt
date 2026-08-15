cmake_minimum_required(VERSION 3.14)

project(weftTests LANGUAGES C ASM)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(weft REQUIRED)
  enable_testing()
endif()

function(add_weft_test NAME)
  add_executable("${NAME}" "source/${NAME}.c")
  target_link_libraries("${NAME}" PRIVATE weft::weft)
  target_include_directories("${NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/source")
  target_compile_features("${NAME}" PRIVATE c_std_11)
  add_test(NAME "${NAME}" COMMAND "${NAME}")
  set_tests_properties("${NAME}" PROPERTIES TIMEOUT 120)
endfunction()

function(add_weft_internal_test NAME)
  add_executable("${NAME}" "source/${NAME}.c" $<TARGET_OBJECTS:weft_objects>)
  target_link_libraries("${NAME}" PRIVATE weft_objects)
  target_include_directories("${NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/source")
  target_compile_definitions("${NAME}" PRIVATE _GNU_SOURCE)
  target_compile_features("${NAME}" PRIVATE c_std_11)
  add_test(NAME "${NAME}" COMMAND "${NAME}")
  set_tests_properties("${NAME}" PROPERTIES TIMEOUT 120)
endfunction()

add_weft_test(test_version)
add_weft_internal_test(test_coro)

add_weft_internal_test(test_coro_guard)
set_tests_properties(test_coro_guard PROPERTIES WILL_FAIL TRUE)

add_folders(Test)
