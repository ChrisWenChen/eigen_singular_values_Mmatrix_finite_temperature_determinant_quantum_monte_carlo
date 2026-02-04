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
set(BLAS_BACKEND "" CACHE STRING "BLAS/LAPACK backend: MKL|OPENBLAS|ACCELERATE (empty for auto)")
set_property(CACHE BLAS_BACKEND PROPERTY STRINGS MKL OPENBLAS ACCELERATE)

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

add_library(core STATIC
  src/core/model_params.cpp
  src/core/hs_field.cpp
  src/core/sparse_csr.cpp
  src/core/kinetic_operator.cpp
)
target_link_libraries(core PUBLIC project_options project_warnings)
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# BLAS/LAPACK backend selection
add_library(core_linalg INTERFACE)

set(_LINALG_BACKEND_NAME "")
set(_LINALG_BACKEND_VENDOR "")

function(_try_blas_backend backend vendor)
  set(BLA_VENDOR "${vendor}" CACHE STRING "BLAS vendor" FORCE)
  unset(BLAS_FOUND CACHE)
  unset(LAPACK_FOUND CACHE)
  find_package(BLAS)
  find_package(LAPACK)
  if(BLAS_FOUND AND LAPACK_FOUND)
    set(_LINALG_BACKEND_NAME "${backend}" PARENT_SCOPE)
    set(_LINALG_BACKEND_VENDOR "${vendor}" PARENT_SCOPE)
  endif()
endfunction()

if(BLAS_BACKEND)
  string(TOUPPER "${BLAS_BACKEND}" _BLAS_BACKEND_UPPER)
  if(_BLAS_BACKEND_UPPER STREQUAL "MKL")
    set(BLA_VENDOR "Intel10_64lp" CACHE STRING "BLAS vendor" FORCE)
    set(_LINALG_BACKEND_NAME "MKL")
  elseif(_BLAS_BACKEND_UPPER STREQUAL "OPENBLAS")
    set(BLA_VENDOR "OpenBLAS" CACHE STRING "BLAS vendor" FORCE)
    set(_LINALG_BACKEND_NAME "OpenBLAS")
  elseif(_BLAS_BACKEND_UPPER STREQUAL "ACCELERATE")
    set(BLA_VENDOR "Apple" CACHE STRING "BLAS vendor" FORCE)
    set(_LINALG_BACKEND_NAME "Accelerate")
  else()
    message(FATAL_ERROR "Unknown BLAS_BACKEND='${BLAS_BACKEND}'. Use MKL, OPENBLAS, or ACCELERATE.")
  endif()

  find_package(BLAS REQUIRED)
  find_package(LAPACK REQUIRED)
else()
  if(APPLE)
    _try_blas_backend("Accelerate" "Apple")
    if(NOT _LINALG_BACKEND_NAME)
      _try_blas_backend("OpenBLAS" "OpenBLAS")
    endif()
  else()
    _try_blas_backend("MKL" "Intel10_64lp")
    if(NOT _LINALG_BACKEND_NAME)
      _try_blas_backend("OpenBLAS" "OpenBLAS")
    endif()
  endif()

  if(NOT _LINALG_BACKEND_NAME)
    message(FATAL_ERROR "No suitable BLAS/LAPACK backend found. Set -DBLAS_BACKEND=MKL|OPENBLAS|ACCELERATE.")
  endif()
endif()

message(STATUS "BLAS/LAPACK backend: ${_LINALG_BACKEND_NAME}")
target_link_libraries(core_linalg INTERFACE BLAS::BLAS LAPACK::LAPACK)
target_compile_definitions(core_linalg INTERFACE
  LINALG_BACKEND_NAME="${_LINALG_BACKEND_NAME}"
  $<$<STREQUAL:${_LINALG_BACKEND_NAME},MKL>:LINALG_BACKEND_MKL=1>
  $<$<STREQUAL:${_LINALG_BACKEND_NAME},OpenBLAS>:LINALG_BACKEND_OPENBLAS=1>
  $<$<STREQUAL:${_LINALG_BACKEND_NAME},Accelerate>:LINALG_BACKEND_ACCELERATE=1>
)

add_executable(bench_dense_matrix apps/bench_dense_matrix.cpp)
add_executable(bench_dense_operator apps/bench_dense_operator.cpp)
add_executable(blas_sanity apps/blas_sanity.cpp)

target_link_libraries(bench_dense_matrix PRIVATE MPI::MPI_CXX core core_linalg project_options project_warnings)
target_link_libraries(bench_dense_operator PRIVATE MPI::MPI_CXX core core_linalg project_options project_warnings)
target_link_libraries(blas_sanity PRIVATE core core_linalg project_options project_warnings)

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

  target_link_libraries(bench_petsc_matrix PRIVATE MPI::MPI_CXX core PkgConfig::PETSC PkgConfig::SLEPC project_options project_warnings)
  target_link_libraries(bench_petsc_shell PRIVATE MPI::MPI_CXX core PkgConfig::PETSC PkgConfig::SLEPC project_options project_warnings)

  target_compile_definitions(bench_petsc_matrix PRIVATE PROJECT_BACKEND_PETSC=1)
  target_compile_definitions(bench_petsc_shell PRIVATE PROJECT_BACKEND_PETSC=1)
endif()

enable_testing()
add_executable(test_hsfield tests/test_hsfield.cpp)
target_link_libraries(test_hsfield PRIVATE core project_options project_warnings)
add_test(NAME test_hsfield COMMAND test_hsfield)
