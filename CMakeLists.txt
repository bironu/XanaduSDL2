cmake_minimum_required(VERSION 3.16)
project(xanadu CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# ソースをsrc直下に置いたままの build ディレクトリでの
# in-source ビルドを防止する
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR
    "In-source builds are not allowed. Please build in a separate "
    "directory, e.g.:\n  mkdir build && cd build && cmake .. && cmake --build .")
endif()

# SDL2版フロントエンドの依存ライブラリ
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

add_subdirectory(src)
