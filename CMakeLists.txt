cmake_minimum_required(VERSION 3.16)

# Compiler preference: macOS -> clang++, Linux -> icpx, fallback -> g++
if(NOT CMAKE_CXX_COMPILER)
  if(APPLE)
    find_program(_PREFERRED_CXX NAMES clang++)
  else()
    find_program(_PREFERRED_CXX NAMES icpx)
  endif()
  if(NOT _PREFERRED_CXX)
    find_program(_PREFERRED_CXX NAMES g++)
  endif()
  if(_PREFERRED_CXX)
    set(CMAKE_CXX_COMPILER "${_PREFERRED_CXX}" CACHE FILEPATH "C++ compiler" FORCE)
  endif()
endif()

project(eigen_singular_values_Mmatrix_finite_temperature_determinant_quantum_monte_carlo
  VERSION 0.1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_PETSC "Enable PETSc/SLEPc backends" ON)

# Prefer Intel MPI wrapper on Linux if available, else mpicxx
if(NOT MPI_CXX_COMPILER)
  if(UNIX AND NOT APPLE)
    find_program(_PREFERRED_MPICXX NAMES mpiicpx mpicxx)
  else()
    find_program(_PREFERRED_MPICXX NAMES mpicxx)
  endif()
  if(_PREFERRED_MPICXX)
    set(MPI_CXX_COMPILER "${_PREFERRED_MPICXX}" CACHE FILEPATH "MPI C++ compiler wrapper" FORCE)
  endif()
endif()

find_package(MPI REQUIRED)

add_library(project_warnings INTERFACE)

target_compile_features(project_warnings INTERFACE cxx_std_17)

add_library(project_options INTERFACE)

# Common include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(bench_dense_matrix apps/bench_dense_matrix.cpp)
add_executable(bench_dense_operator apps/bench_dense_operator.cpp)

target_link_libraries(bench_dense_matrix PRIVATE MPI::MPI_CXX project_options project_warnings)
target_link_libraries(bench_dense_operator PRIVATE MPI::MPI_CXX project_options project_warnings)

target_compile_definitions(bench_dense_matrix PRIVATE PROJECT_BACKEND_DENSE=1)
target_compile_definitions(bench_dense_operator PRIVATE PROJECT_BACKEND_DENSE=1)

if(ENABLE_PETSC)
  # Allow environment variables to guide PETSc/SLEPc discovery
  if(NOT PETSC_DIR AND DEFINED ENV{PETSC_DIR})
    set(PETSC_DIR "$ENV{PETSC_DIR}" CACHE PATH "PETSc directory" FORCE)
  endif()
  if(NOT PETSC_ARCH AND DEFINED ENV{PETSC_ARCH})
    set(PETSC_ARCH "$ENV{PETSC_ARCH}" CACHE STRING "PETSc architecture" FORCE)
  endif()
  if(NOT SLEPC_DIR AND DEFINED ENV{SLEPC_DIR})
    set(SLEPC_DIR "$ENV{SLEPC_DIR}" CACHE PATH "SLEPc directory" FORCE)
  endif()

  # Use pkg-config to locate PETSc/SLEPc and create imported targets
  if(PETSC_DIR AND PETSC_ARCH)
    set(ENV{PKG_CONFIG_PATH}
        "${PETSC_DIR}/${PETSC_ARCH}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
  endif()
  if(SLEPC_DIR)
    set(ENV{PKG_CONFIG_PATH}
        "${SLEPC_DIR}/${PETSC_ARCH}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
  endif()

  find_package(PkgConfig REQUIRED)
  pkg_check_modules(PETSC REQUIRED IMPORTED_TARGET petsc PETSc)
  pkg_check_modules(SLEPC REQUIRED IMPORTED_TARGET slepc SLEPc)

  add_executable(bench_petsc_matrix apps/bench_petsc_matrix.cpp)
  add_executable(bench_petsc_shell apps/bench_petsc_shell.cpp)

  target_link_libraries(bench_petsc_matrix PRIVATE MPI::MPI_CXX PkgConfig::PETSC PkgConfig::SLEPC project_options project_warnings)
  target_link_libraries(bench_petsc_shell PRIVATE MPI::MPI_CXX PkgConfig::PETSC PkgConfig::SLEPC project_options project_warnings)

  target_compile_definitions(bench_petsc_matrix PRIVATE PROJECT_BACKEND_PETSC=1)
  target_compile_definitions(bench_petsc_shell PRIVATE PROJECT_BACKEND_PETSC=1)
endif()
