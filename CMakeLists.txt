project(scorep_plugin_mpi)

cmake_minimum_required(VERSION 3.8)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/common")

include(scorep_plugin_common/DefaultBuildType.cmake)

add_subdirectory(scorep_plugin_cxx_wrapper)

add_library(scorep_plugin_mpi
            SHARED
            src/scorep_plugin_mpi.cpp
            #src/pfm_wrapper.cpp
            #src/ring_buffer.cpp
            #src/perf_sampling.cpp
            src/utils.cpp
            src/mpi_t_sampling/mpi_t_sampling.cpp
            src/mpi_t_sampling/mpi_hooks.c
            src/mpi_t_sampling/mpi_t_sampling_overrides.cpp
            src/mpi_t_sampling/utility.c)

set_target_properties(scorep_plugin_mpi PROPERTIES CXX_STANDARD 17)

target_include_directories(scorep_plugin_mpi PRIVATE
  src 
  include
  src/mpi_t_sampling)

target_compile_options(scorep_plugin_mpi INTERFACE -Wall -pedantic -Wextra -fPIC)

target_link_libraries(scorep_plugin_mpi PRIVATE 
  Scorep::scorep-plugin-cxx)

install(TARGETS scorep_plugin_mpi DESTINATION lib)

