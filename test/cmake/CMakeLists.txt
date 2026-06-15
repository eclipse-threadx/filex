cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0057 NEW)

# Suppress try_compile link step when MSVC cannot produce a runnable executable
# (e.g. when the VS environment is not fully initialised for a given target arch).
if((DEFINED THREADX_ARCH) AND ((THREADX_ARCH STREQUAL "win32") OR (THREADX_ARCH STREQUAL "win64")))
  set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
endif()

project(filex_test LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Set build configurations
set(BUILD_CONFIGURATIONS
    default_build_coverage no_cache_build no_cache_standalone_build
    fault_tolerant_build_coverage no_check_build no_cache_fault_tolerant_build
    standalone_build_coverage standalone_fault_tolerant_build_coverage
    standalone_no_cache_fault_tolerant_build)
set(CMAKE_CONFIGURATION_TYPES
    ${BUILD_CONFIGURATIONS}
    CACHE STRING "list of supported configuration types" FORCE)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
                                             ${CMAKE_CONFIGURATION_TYPES})
list(GET CMAKE_CONFIGURATION_TYPES 0 BUILD_TYPE)
if((NOT CMAKE_BUILD_TYPE) OR (NOT ("${CMAKE_BUILD_TYPE}" IN_LIST
                                   CMAKE_CONFIGURATION_TYPES)))
  set(CMAKE_BUILD_TYPE
      "${BUILD_TYPE}"
      CACHE STRING "Build Type of the project" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")
set(FX_FAULT_TOLERANT_DEFINITIONS
    -DFX_ENABLE_FAULT_TOLERANT -DFX_UPDATE_FILE_SIZE_ON_ALLOCATE
    -DFX_FAULT_TOLERANT_TRANSACTION_FAIL_FUNCTION)
set(default_build_coverage "")
set(no_cache_build -DFX_DISABLE_CACHE -DFX_DISABLE_FAT_ENTRY_REFRESH)
set(fault_tolerant_build_coverage ${FX_FAULT_TOLERANT_DEFINITIONS})
set(standalone_build_coverage -DFX_STANDALONE_ENABLE)
set(standalone_fault_tolerant_build_coverage ${FX_FAULT_TOLERANT_DEFINITIONS}
                                             -DFX_STANDALONE_ENABLE)
set(no_cache_standalone_build -DFX_DISABLE_CACHE -DFX_STANDALONE_ENABLE)
set(no_check_build -DFX_DISABLE_ERROR_CHECKING)
set(no_cache_fault_tolerant_build ${no_cache_build} ${FX_FAULT_TOLERANT_DEFINITIONS})
set(standalone_no_cache_fault_tolerant_build ${no_cache_build} ${FX_FAULT_TOLERANT_DEFINITIONS} -DFX_STANDALONE_ENABLE)

if(MSVC)
  add_compile_options(/W3 /Zi)
  add_link_options(/DEBUG /INCREMENTAL:NO)
  # Both cl.exe and gcc accept -DFOO; use it to keep the per-configuration
  # definitions consistent with the Linux path.
  add_compile_options(-DFX_REGRESSION_TEST ${${CMAKE_BUILD_TYPE}})
else()
  add_compile_options(
    -m32
    -ggdb
    -g3
    -gdwarf-2
    -fdiagnostics-color
    -Werror
    -DFX_REGRESSION_TEST
    ${${CMAKE_BUILD_TYPE}})
  add_link_options(-m32)
endif()

if(CMAKE_BUILD_TYPE MATCHES ".*standalone.*")
  set(FX_STANDALONE_ENABLE
      ON
      CACHE BOOL "Enable Filex in standalone mode")
endif()

enable_testing()

# ThreadX dependency: on Windows build from source; on Linux use run.sh.
if(MSVC)
  if(NOT FX_STANDALONE_ENABLE)
    if(NOT DEFINED THREADX_SOURCE_DIR)
      message(FATAL_ERROR
        "THREADX_SOURCE_DIR must point to a ThreadX source tree for non-standalone Windows builds. "
        "Pass -DTHREADX_SOURCE_DIR=<path> to cmake, or use build_fx.ps1 which sets it automatically.")
    endif()
    add_subdirectory(${THREADX_SOURCE_DIR} threadx_build)
  endif()
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../.. filex)
add_subdirectory(regression)
add_subdirectory(samples)

# Coverage instrumentation (GCC/Clang only — not supported by MSVC)
if(NOT MSVC AND CMAKE_BUILD_TYPE MATCHES ".*_coverage")
  target_compile_options(filex PRIVATE -fprofile-arcs -ftest-coverage)
  target_link_options(filex PRIVATE -fprofile-arcs -ftest-coverage)
endif()

# Build ThreadX shared library for Linux
if(NOT MSVC AND NOT FX_STANDALONE_ENABLE)
    execute_process(COMMAND ${CMAKE_CURRENT_LIST_DIR}/run.sh build_libs)
    add_custom_target(build_libs ALL COMMAND ${CMAKE_CURRENT_LIST_DIR}/run.sh
                                             build_libs)
    add_dependencies(filex build_libs)
    target_include_directories(filex PUBLIC ${CMAKE_BINARY_DIR}/../libs/inc)
    add_library(threadx SHARED IMPORTED GLOBAL)
    add_library("azrtos::threadx" ALIAS threadx)
    set_target_properties(
      threadx PROPERTIES IMPORTED_LOCATION
                         ${CMAKE_BINARY_DIR}/../libs/threadx/libthreadx.so)
endif()

if(MSVC)
  target_compile_options(filex PRIVATE /W3)
else()
  target_compile_options(
    filex
    PRIVATE -Werror
            -Wall
            -Wextra
            -pedantic
            -fmessage-length=0
            -fsigned-char
            -ffunction-sections
            -fdata-sections
            -Wunused
            -Wuninitialized
            -Wmissing-declarations
            -Wconversion
            -Wpointer-arith
            -Wshadow
            -Wlogical-op
            -Waggregate-return
            -Wfloat-equal)
endif()

