cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    weft
    VERSION 0.1.0
    DESCRIPTION "Coroutine based async runtime"
    HOMEPAGE_URL "https://getweft.com"
    LANGUAGES C ASM
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Dependencies ----

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(uring REQUIRED IMPORTED_TARGET liburing>=2.3)

# ---- Generated headers ----

set(WEFT_DEFAULT_STACK_SIZE 65536 CACHE STRING "Default coroutine stack size in bytes")
set(WEFT_DEFAULT_RING_ENTRIES 4096 CACHE STRING "Default io_uring SQ entries")
set(WEFT_CACHELINE_SIZE 64 CACHE STRING "Cache line size in bytes")

include(CheckSymbolExists)
include(CheckIncludeFile)

set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
check_symbol_exists(memfd_create "sys/mman.h" WEFT_HAVE_MEMFD_CREATE)
unset(CMAKE_REQUIRED_DEFINITIONS)

check_include_file("sanitizer/common_interface_defs.h" WEFT_HAVE_SANITIZER_INTERFACE)

configure_file(
    "${PROJECT_SOURCE_DIR}/include/weft/version.h.in"
    "${PROJECT_BINARY_DIR}/export/weft/version.h"
    @ONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/source/config.h.in"
    "${PROJECT_BINARY_DIR}/private/config.h"
    @ONLY
)

# ---- Object library shared by the library and the internal tests ----

set(COMPILER_HAS_HIDDEN_VISIBILITY ON CACHE INTERNAL "")
set(COMPILER_HAS_HIDDEN_INLINE_VISIBILITY ON CACHE INTERNAL "")
set(COMPILER_HAS_DEPRECATED_ATTR ON CACHE INTERNAL "")

add_library(
    weft_objects OBJECT
    source/runtime.c
    source/coro.c
    source/coro_x86_64.S
)

target_include_directories(
    weft_objects PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/source"
    "${PROJECT_BINARY_DIR}/private"
    "${PROJECT_BINARY_DIR}/export"
)

target_compile_definitions(weft_objects PRIVATE _GNU_SOURCE weft_weft_EXPORTS)
target_compile_features(weft_objects PUBLIC c_std_11)
target_link_libraries(weft_objects PUBLIC PkgConfig::uring Threads::Threads)

set_target_properties(
    weft_objects PROPERTIES
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    POSITION_INDEPENDENT_CODE ON
)

# ---- Declare library ----

add_library(weft_weft $<TARGET_OBJECTS:weft_objects>)
add_library(weft::weft ALIAS weft_weft)

include(GenerateExportHeader)
generate_export_header(
    weft_weft
    BASE_NAME weft
    EXPORT_FILE_NAME export/weft/weft_export.h
    EXPORT_MACRO_NAME WEFT_API
    NO_EXPORT_MACRO_NAME WEFT_LOCAL
    DEPRECATED_MACRO_NAME WEFT_DEPRECATED
)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(weft_objects PUBLIC WEFT_STATIC_DEFINE)
  target_compile_definitions(weft_weft PUBLIC WEFT_STATIC_DEFINE)
endif()

set_target_properties(
    weft_weft PROPERTIES
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    POSITION_INDEPENDENT_CODE ON
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME weft
    OUTPUT_NAME weft
)

target_include_directories(
    weft_weft ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    weft_weft SYSTEM
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(weft_weft PUBLIC c_std_11)
target_link_libraries(weft_weft PRIVATE PkgConfig::uring Threads::Threads)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(
      weft_weft PRIVATE
      "LINKER:--version-script=${PROJECT_SOURCE_DIR}/source/weft.map"
      "LINKER:--no-undefined"
  )
endif()

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Examples ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_EXAMPLES "Build examples tree." "${weft_DEVELOPER_MODE}")
  if(BUILD_EXAMPLES)
    add_subdirectory(example)
  endif()
endif()

# ---- Developer mode ----

if(NOT weft_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of weft"
  )
endif()

include(cmake/dev-mode.cmake)
