cmake_minimum_required(VERSION 3.14)

project(weftTests LANGUAGES C)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(weft REQUIRED)
  enable_testing()
endif()

# ---- Tests ----

add_executable(weft_test source/test_version.c)
target_link_libraries(weft_test PRIVATE weft::weft)
target_compile_features(weft_test PRIVATE c_std_11)

add_test(NAME weft_test COMMAND weft_test)

# ---- End-of-file commands ----

add_folders(Test)
