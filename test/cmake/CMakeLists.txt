cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0057 NEW)

project(threadx_test LANGUAGES C)

# Set build configurations
set(BUILD_CONFIGURATIONS
    default_build_coverage no_cache_build no_cache_standalone_build
    fault_tolerant_build_coverage no_check_build no_cache_fault_tolerant_build
    standalone_build_coverage standalone_fault_tolerant_build_coverage 
    standalone_no_cache_fault_tolerant_build trace_build_coverage
    lba_64bit_build_coverage)
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
set(standalone_build_coverage  -DFX_STANDALONE_ENABLE)
set(standalone_fault_tolerant_build_coverage ${FX_FAULT_TOLERANT_DEFINITIONS}
                                             -DFX_STANDALONE_ENABLE)       
set(no_cache_standalone_build -DFX_DISABLE_CACHE -DFX_STANDALONE_ENABLE)
set(no_check_build ${FX_COMPILE_DEFINITIONS} -DFX_DISABLE_ERROR_CHECKING)
set(no_cache_fault_tolerant_build ${no_cache_build} ${FX_FAULT_TOLERANT_DEFINITIONS})
set(standalone_no_cache_fault_tolerant_build ${no_cache_build} ${FX_FAULT_TOLERANT_DEFINITIONS} -DFX_STANDALONE_ENABLE)

# TX_ENABLE_EVENT_TRACE is what fx_api.h tests before remapping FX_TRACE_IN_LINE_INSERT,
# FX_TRACE_EVENT_UPDATE, FX_TRACE_OBJECT_REGISTER and FX_TRACE_OBJECT_UNREGISTER onto the
# ThreadX trace services. None of the configurations above defines it, so those macros
# expand to nothing in all of them: the trace call sites throughout common/src and the
# whole body of the four fx_trace_*.c shims are absent from the coverage denominator
# rather than uncovered in it.
#
# It must not be combined with FX_STANDALONE_ENABLE. fx_api.h gates the remapping on
# TX_ENABLE_EVENT_TRACE && !FX_STANDALONE_ENABLE, so a standalone trace configuration
# would compile exactly what the standalone configurations already compile.
set(trace_build_coverage -DTX_ENABLE_EVENT_TRACE)

# FX_DRIVER_USE_64BIT_LBA widens fx_media_hidden_sectors and
# fx_media_driver_logical_sector in FX_MEDIA from ULONG to ULONG64, so a media driver can
# address a volume past the 32-bit sector limit. None of the configurations above defines
# it, which leaves eleven assignments in the sector read, write and flush paths compiled
# out of every build: each site assigns the logical sector to the driver twice over, once
# narrowed to ULONG and once not, and only the narrowed arm was ever compiled.
#
# It is a supported configuration of the shipped product rather than an internal switch --
# fx_user_sample.h offers it to applications -- and it changes the layout of a public
# structure, so the whole tree including the regression suite's own driver is built with
# it rather than the library alone.
#
# It carries the fault-tolerant macros as well, and not for the fault-tolerant code's own
# sake: one of the eleven assignments, in _fx_utility_logical_sector_write, sits inside
# #ifdef FX_FAULT_TOLERANT, so it is reached only by a build that defines both. Without
# them this configuration compiles ten of the eleven and the last needs a combination no
# other configuration offers.
set(lba_64bit_build_coverage ${FX_FAULT_TOLERANT_DEFINITIONS} -DFX_DRIVER_USE_64BIT_LBA)

add_compile_options(
  -m32
  -std=c99
  -ggdb
  -g3
  -gdwarf-2
  -fdiagnostics-color
  -Werror
  -DFX_REGRESSION_TEST
  ${${CMAKE_BUILD_TYPE}})
add_link_options(-m32)

if(CMAKE_BUILD_TYPE MATCHES ".*standalone.*")
  set(FX_STANDALONE_ENABLE
      ON
      CACHE BOOL "Enable Filex in standalone mode")
endif()

enable_testing()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../.. filex)
add_subdirectory(regression)
add_subdirectory(samples)

# Coverage
#
# The gate here used to be the build type alone, and only four of the nine configurations
# then defined had a name matching "*_coverage". So five configurations built and ran
# every test and their coverage was thrown away. That is not redundancy discarded: each
# configuration selects a different set of FX_ feature macros, so the code the other five
# compile is *absent from the denominator* rather than reported as uncovered. A figure
# taken from one of them is the executable-line count of one configuration, not of FileX.
#
# TX_COVERAGE instruments a build regardless of its name. It defaults to OFF and the
# build-type match is kept, so configuring a single configuration by hand behaves exactly
# as it did before; the regression run turns it on for all eleven and merges the results.
# The name match cannot simply be dropped: the shared runner decides which configurations
# to collect from by the same suffix, so a build instrumented by name with no collection
# behind it, or a collection with no instrumentation behind it, are both reachable from
# here.
option(TX_COVERAGE "Instrument this build for coverage regardless of build type" OFF)
if(TX_COVERAGE OR CMAKE_BUILD_TYPE MATCHES ".*_coverage")
  target_compile_options(filex PRIVATE -fprofile-arcs -ftest-coverage)

  # PUBLIC on the link options, PRIVATE on the compile options above. Only common/src is
  # certified source and only it needs instrumenting, but every consumer that links filex
  # needs the gcov runtime on its own link line. PRIVATE here happens to work while
  # BUILD_SHARED_LIBS is ON, because the runtime is then linked into libfilex.so and the
  # test executables resolve __gcov_init through it; with a static library the same tests
  # fail to link with undefined __gcov_init, __gcov_exit and __gcov_merge_add. The flag
  # that decides which of those applies is set in the shared runner, not here.
  target_link_options(filex PUBLIC -fprofile-arcs -ftest-coverage)
endif()

# Build ThreadX library once
if(NOT FX_STANDALONE_ENABLE)
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

