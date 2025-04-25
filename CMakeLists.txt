cmake_minimum_required(VERSION 3.14)
project(GraphColoring LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# Choose Release by default with high optimization
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Fetch Eigen3
include(FetchContent)
FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 3.4.0
)
FetchContent_MakeAvailable(eigen)

# OpenMP (optional)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  message(STATUS "Found OpenMP, enabling")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Vectorization flags
if(MSVC)
  add_compile_options(/arch:AVX2)
else()
  add_compile_options(-march=native -ffast-math)
endif()

# Threads
find_package(Threads REQUIRED)

# Executable
add_executable(graph_coloring
  src/main.cpp
  src/algorithms/GreedyColoring.cpp
  src/algorithms/DSaturColoring.cpp
  src/algorithms/GeneticColoring.cpp
)

# Include directories
target_include_directories(graph_coloring PRIVATE
  ${PROJECT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(graph_coloring PRIVATE
  Eigen3::Eigen
  Threads::Threads
  $<$<BOOL:${OpenMP_CXX_FOUND}>:OpenMP::OpenMP_CXX>
)
